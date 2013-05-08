require( "ovel.help" )

itext = {}
otext = {}
secw = {}
istb = {}
ideState = {}



function devbtn()
  print( "BOOOMMMMM!!!!\n\n\n" );
end

function arrange() 
  if ideState.outputSticky then
  local sa = secw.area
  local ma = mainw.area
  sa.x = ma.x + ma.width

  sa.y = ma.y
  sa.height = ma.height
  secw.area = sa
  secw:raise()
  end
  
end

function inputpos()
  local ts = itext.selection
  local cl = itext:lineFromChar( ts.front )
  local cp = ts.front - itext:charFromLine( cl )
  if ts.front == ts.back then
    return ovel.stripPath( ideState.filename or "<no file>" ) .. ":" .. cl .. ":" .. cp .. ( ideState.modified and "*" or " " )
  else
    local clb = itext:lineFromChar( ts.back )
    local cpb = ts.back - itext:charFromLine( clb )
    return ovel.stripPath( ideState.filename or "<no file>" ) .. ":" .. cl .. ":" .. cp .. "-" .. clb .. ":" .. cpb .. ( ideState.modified and "*" or " " ) 
  end 
end

function redrawMainw()
  local ta = mainw.childArea; ta.x = 0; ta.y = 0
  local t = istb.area
  local ph
  if t then 
    ph = t.height
  else
    ph = 10
  end
  ta.height = ta.height - ph
  itext.area = ta
  ta.y = ta.height
  ta.height = ph
  istb.area = ta
  if istb.setParts then
    istb:setParts( { [ta.width] = inputpos() } )
  end
end

do 
local os = true
local fn = nil
local maxRecentFiles = 5
local recentFiles = {}
local function resetRecentFiles()
  if #recentFiles ~= 0 then
    for i = 1, maxRecentFiles do mainw:removeMenu( 1200 + i ) end
    for i = #recentFiles, 1, -1 do mainw:addMenu( recentFiles[ i ], 1200 + i, 120 ) end
    mainw:setMenuItemState( 120, ( ( #recentFiles == 0 and "disabled" ) or "unchecked" ) )
    redrawMainw()
  end
end
setmetatable( ideState, {
__index = function( self, key )
  if key == "modified" then 
    return itext.modified
  elseif key == "outputSticky" then
    return os
  elseif key == "outputVisibility" then
    return secw.visibility
  elseif key == "filename" then
    return fn
  elseif key == "state" then
    local ans = { [ "outputSticky" ] = os, [ "outputVisibility" ] = secw.visibility, [ "mainArea" ] = mainw.area, [ "outputArea" ] = secw.area, [ "helpState" ] = ovel.help.state() }
    if fn ~= nil then ans.filename = fn end
    ans.recentFiles = recentFiles
    return ans
  elseif key == "stateString" then
    return ovel.serialCompress( ideState.state )
  elseif key == "saveAs"
    then return function( self )
      local fp
      fp = ovel.fileDialog( { ["save"] = true } )
      if fp == nil then ovel.inform"No file name specified, not saving file."; return end
      local of = ovel.getFullPath( fp )
      if of ~= nil then
        if not ovel.query( "File " .. of .. "\nexists, overwrite it?" ) then return end
      end         
      if not ovel.saveFile( fp, itext.text, true ) 
        then ovel.inform( "Failed to save file:\n" .. fp )  
      else
        fn = fp
        mainw:setMenuItemState( 112, "unchecked" )
        itext.modified = false
        redrawMainw()
      end
    end
  elseif key == "recentFiles" then
    return recentFiles
  elseif key == "save" then
    return function( self )
      if self.modified then
        if fn ~= nil then
          if ovel.saveFile( fn, itext.text, true ) then 
            itext.modified = false
            redrawMainw()
          else
            ovel.inform( "Failed to save file:\n" .. fn )  
          end
        else
          self:saveAs()
        end
      end
    end
  elseif key == "quit" then 
    return function( self )
      if self.modified and ovel.query"Buffer moddified and not saved, save before quiting?" then
        self:save()
      end
      ovel.userData.ideState = self.stateString
      ovel.quit()
    end
  elseif key == "loadStateString" then
    return function( self, value )
      local ns
      local suc = true
      if type( value ) == "string" and value ~= "" then
        ns = ovel.serialUncompress( value )
      end
      if ns == nil then return false end
      if type( ns.outputSticky ) ~= "boolean" then fail(); return false end
      if type( ns.outputVisibility ) ~= "boolean" then return false end
      if ns.filename and type( ns.filename ) ~= "string" then return false end
      
      local function ta( tb ) 
        local sa = ovel.sysInfoVirtualDisplay()
        if type( tb ) ~= "table" or type( tb.x ) ~= "number" or type( tb.y ) ~= "number" or type( tb.width ) ~= "number" or type( tb.height ) ~= "number" then return false end
        if tb.x < 0 or tb.x > sa.width or tb.y < 0 or tb.y > sa.height then return false end
        return true
      end
      if not ( ta( ns.mainArea ) and ta( ns.outputArea ) ) then return false end

      if ns.filename then
        local cfn = ovel.getFullPath( ns.filename )
        if cfn then 
          self.filename = cfn 
        else
          ovel.inform( "Error loading previously loaded file\n" .. ns.filename )
        end
      end
      if type( ns.recentFiles ) ~= "table" or #ns.recentFiles > maxRecentFiles then return false end
      for i = 1, #ns.recentFiles do if type( ns.recentFiles[ i ] ) ~= "string" then return false end end
      recentFiles = {}
      for i = 1, #ns.recentFiles do recentFiles[ i ] = ns.recentFiles[ i ] end
      resetRecentFiles()
      mainw.area = ns.mainArea
      secw.area = ns.outputArea
      self.outputVisibility = ns.outputVisibility
      self.outputSticky = ns.outputSticky
      if type( ns.helpState ) == "table" then
        ovel.help.loadState( ns.helpState )
      end
      return true
    end
  else
    ovel.error( "Invalid key query in ideState.", 2 )
  end
end,
__newindex = function( self, key, value )
  if key == "filename" then
    if self.modified and ovel.query"Buffer moddified and not saved, save buffer before opening new file?" then
      self:save()
    end     
    if value == nil or type( value ) ~= "string" then 
      fn = nil
      itext.text = ""
      mainw:setMenuItemState( 112, "disabled" )
      itext.modified = false;
      redrawMainw()
    else
      local fc = ovel.loadFile( value )
      if fc then 
        itext.text = fc
        fn = value
        mainw:setMenuItemState( 112, "unchecked" )
        itext.modified = false;
        for i = 0, #recentFiles do
          if recentFiles[ i ] == value then table.remove( recentFiles, i ) end
        end
        table.insert( recentFiles, 1, value )
        while #recentFiles > maxRecentFiles do table.remove( recentFiles ) end       
        resetRecentFiles()
      else
        ovel.inform( "No such file\n" .. value )
      end
    end
  elseif key == "outputSticky" then
    os = value
    mainw:setMenuItemState( 132, ( os and "" or "un" ) .. "checked" )
  elseif key == "outputVisibility" then
    secw.visibility = value
    mainw:setMenuItemState( 133, ( value and "" or "un" ) .. "checked" )    
  else
    ovel.error( "Invalid key assignment in ideState.", 2 )
  end
end
} )

end


function domenu( obj, item )
  if item == 111 then
    ideState:quit()
  elseif item == 112 then
    if ideState.filename ~= nil then ideState.filename = nil end
  elseif item == 113 then
    local nf = ovel.fileDialog()
    if nf then ideState.filename = nf end
  elseif item == 114 then
    ideState:saveAs()
  elseif item == 115 then
    ideState:save()
  elseif item == 121 and #itext.text > 0 then
    ovel.thread( itext.text )    
  elseif item > 1200 and item < 1299 then
    ideState.filename = ideState.recentFiles[ item - 1200 ]
  elseif item == 122 then
    obj:selectAll()
  elseif item == 123 then
    obj:replaceSelection( ovel.getPasteBuffer() )
  elseif item == 124 and ( #obj.selectionText > 0 ) then
    ovel.setPasteBuffer( obj.selectionText )
  elseif item == 125 and ( #obj.selectionText > 0 ) then
    ovel.setPasteBuffer( obj.selectionText )
    obj:replaceSelection( "" )
  elseif item == 126 then
    obj:undo()
  elseif item == 127 then
    obj:redo()
  elseif item == 131 then
    loop( "c" )
  elseif item == 132 then
    ideState.outputSticky = not ideState.outputSticky
    arrange()
  elseif item == 133 then
    ideState.outputVisibility = not ideState.outputVisibility
  elseif item == 134 then
    ovel.setPasteBuffer( otext.text )
  elseif item == 141 then
    print( "Vislib version: " .. ovel.sysInfoVislibVersion() .. "\n" )
    print( "Operating System: " .. ovel.sysInfoOsVersion() .. "\n" )
    print( "Lua version: " .. ovel.sysInfoLuaVersion() .. "\n\n" )
    local da = ovel.sysInfoDesktop()
    local sa = ovel.sysInfoVirtualDisplay()
    print( "Desktop: " .. da.width .. "x" .. da.height .. "\n" )
    print( "Virtual display: " .. sa.width .. "x" .. sa.height .. "\n" )
    print"\nDisplays:"
    for _n, m in pairs( ovel.sysInfoDisplays() ) do
      print( "\n  " .. m.display .. ":" .. m.identifier .. ": " .. m.width .. "x" .. m.height .. "@" .. m.frequency .. "h@" .. m.bpp .. "bpp" ) 
    end
  elseif item == 142 then
    ovel.thread"ovel.ide()"
  elseif item == 143 then
    devbtn()
  elseif item == 144 then
    ovel.help.show( "test" )    
  elseif item == 145 then
    ovel.help.show( "license" )    
  end   
end

function maincb( win, mtype, arg1 )
  if mtype == "close" then
    ideState:quit()
  elseif mtype == "menu" then
    domenu( itext, arg1 )
    return false
  elseif mtype == "move" then
    arrange()
  elseif mtype == "resize" or mtype == "maximize" or mtype == "activate" then
    redrawMainw()
    arrange()
  elseif mtype == "keydown" then
    if arg1.name == "Alt+F4" then
      domenu( itext, 111 ); return false
    elseif arg1.name == "Ctrl+O" then
      domenu( itext, 113 ); return false      
    elseif arg1.name == "Ctrl+S" then
      domenu( itext, 115 ); return false      
    elseif arg1.name == "Shift+Enter" or arg1.name == "F5" then
      domenu( itext, 121 ); return false
    elseif arg1.name == "Ctrl+A" then
      domenu( itext, 122 ); return false
    elseif arg1.name == "Ctrl+Z" then
      domenu( itext, 126 ); return false
    elseif arg1.name == "Ctrl+Y" then
      domenu( itext, 127 ); return false
    elseif arg1.name == "Ctrl+V" or arg1.name == "Shift+Insert" then
      domenu( itext, 123 ); return false
    elseif arg1.name == "Ctrl+C" or arg1.name == "Ctrl+Insert" then
      domenu( itext, 124 ); return false
    elseif arg1.name == "Ctrl+X" or arg1.name == "Shift+Delete" then
      domenu( itext, 125 ); return false
    elseif arg1.name == "Ctrl+W" then
      domenu( itext, 131 ); return false
    elseif arg1.name == "Ctrl+Shift+Alt+D" then
      domenu( itext, 143 ); return false
    elseif arg1.name == "F1" then
      domenu( itext, 144 ); return false      
    end
  end
  return true
end  

function seccb( win, mtype, arg1 )
  if mtype == "close" then
    ideState.outputVisibility = false
    return false
  elseif mtype == "move" or mtype == "resize" or mtype == "activate" then
    local ta = win.childArea; ta.x = 0; ta.y = 0
    otext.area = ta
    return true
  elseif mtype == "keydown" then
    if arg1.name == "Ctrl+C" or arg1.name == "Ctrl+Insert" then
      domenu( otext, 124 )
    elseif arg1.name == "Ctrl+A" then
      domenu( otext, 122 )
    elseif arg1.name == "Ctrl+W" then
      domenu( otext, 131 )
    end
    return false
  elseif mtype == "keyup" then
    return false
  end  
  return true
end  

function inputcb( wid, mtype, arg1 )
  if istb.setPartText then 
    local ts = itext.selection
    istb:setPartText( 0, inputpos() )
  end
  if mtype == "keyup" or mtype == "keydown" or mtype == "mouse" then
    return maincb( mainw, mtype, arg1 )
  end
  return true
end

function istbcb( win, mtype, arg1 )
  return true
end


do
function outputcb( win, mtype, arg1 )
  if mtype == "keyup" or mtype == "keydown" then
    return seccb( mainw, mtype, arg1 )
  elseif mtype == "mouse" and arg1.type == "doubleclick" then
    local ln = otext:lineFromChar( arg1.characterPosition )
    local l = ( otext.text ):sub( otext:charFromLine( ln ) + 1, otext:charFromLine( ln + 1 ) )
    _, _, cap = string.find( l, '^%[string "<new thread>"%]:(%d+):' )
    if cap then 
      loop( "g" .. cap )
      return false
    end
  end
  return true
end
end

do
local dsa = ovel.sysInfoDesktop();
mainw = ovel.newwindow( dsa.x + ( dsa.width / 10 ), dsa.y + ( dsa.height / 8 ), dsa.width / 2, dsa.height * 0.666, "Ovel", "app", maincb )
secw = ovel.newwindow( 0, 0, dsa.width / 3, 1, "Output", "toolbox", seccb )
end



mainw:addMenu( "&Help", 14, 0 )
mainw:addMenu( "&About", 145, 14 )
mainw:addMenu( "&Develepor Button\tCtrl+Alt+Shift+D", 143, 14 )
mainw:addMenuBreak( 200003, 14 );
mainw:addMenu( "&System Information", 141, 14 )
mainw:addMenu( "&Ide", 142, 14 )
mainw:addMenu( "&Help\tF1", 144, 14 )


mainw:addMenu( "&Output", 13, 0 )
mainw:addMenu( "&Clear\tCtrl+W", 131, 13 )
mainw:addMenu( "Copy &All", 134, 13 )
mainw:addMenuBreak( 200002, 13 );
mainw:addMenu( "&Sticky", 132, 13 )
mainw:addMenu( "&Visible", 133, 13 )

mainw:addMenu( "&Edit", 12, 0 )
mainw:addMenu( "&Redo\tCtrl+Y", 127, 12 );
mainw:addMenu( "&Undo\tCtrl+Z", 126, 12 );
mainw:addMenuBreak( 200001, 12 );
mainw:addMenu( "&Select All\tCtrl+A", 122, 12 );
mainw:addMenu( "&Paste\tCtrl+V,Shift+Insert", 123, 12 );
mainw:addMenu( "&Copy\tCtrl+C,Ctrl+Insert", 124, 12 );
mainw:addMenu( "Cu&t\tCtrl+X,Shift+Delete", 125, 12 );
mainw:addMenuBreak( 200000, 12 );
mainw:addMenu( "E&valuate\tShift+Enter,F5", 121, 12 )

mainw:addMenu( "&File", 11, 0 )
mainw:addMenu( "E&xit\tAlt+F4", 111, 11 )
mainw:addMenu( "&Close", 112, 11 )
mainw:addMenuBreak( 200010, 11 )
mainw:addMenu( "Recent &Files", 120, 11 )
mainw:setMenuItemState( 120, "disabled" )
mainw:setMenuItemState( 112, "disabled" )
mainw:addMenu( "&Open\tCtrl+O", 113, 11 )
mainw:addMenuBreak( 200011, 11 )
mainw:addMenu( "Save &As...\t", 114, 11 )
mainw:addMenu( "Save\tCtrl+S", 115, 11 )

  
itext = ovel.newwidget( mainw, 0, 0, mainw.childArea.width, mainw.childArea.height, "text", inputcb )
istb = ovel.newwidget( mainw, 0, 0, 0, 0, "statusbar", istbcb )
otext = ovel.newwidget( secw, 0, 0, secw.childArea.width, secw.childArea.height, "textNowrap", outputcb )


local printFontStyle = ovel.fontStyle( ovel.defaultFontStyle )
function loop( itcm )
  local tp = itcm:sub( 1, 1 )
  local txt = itcm:sub( 2 )  
  if tp == "e" then
    assert( loadstring( txt ) )()
  elseif tp == "f" then
    local l = tonumber( txt )
    ovel.itcPost( "printFontStyle" .. printFontStyle:serialize(), l )
  elseif tp == "p" then
    printFontStyle = ovel.fontStyle( txt )
  elseif tp == "g" then
    local l = tonumber( txt )
    itext:select( itext:charFromLine( l ), itext:charFromLine( l + 1 ) - 1 )
    mainw:focus()
    itext:focus()
  elseif tp == "c" then
    otext.text = "";
  elseif tp == "n" and #txt > 0 then 
    otext:append( txt, printFontStyle )
  elseif tp == "i" and #txt > 0 then 
    otext:append( txt, ovel.informationFontStyle ) 
  elseif tp == "w" then
    otext:append( "\n\n" .. txt .. "\n\n", ovel.warningFontStyle )
  end        
end
ovel.inheritance = "ovel.ideThread = " .. tostring( ovel.threadId ) .. "\n";
ovel.ideThread = ovel.threadId










 
arrange()
do
  local ides = ovel.userData.ideState
  if ides then
    if not ideState:loadStateString( ovel.userData.ideState ) then
      ovel.inform( "Failed to load IDE state; using defaults.", "IDE state error" )
      ideState.outputVisibility = true
    end
  else
    ideState.outputVisibility = true
  end
end

ideState.outputSticky = ideState.outputSticky
mainw.visibility = true

redrawMainw()

itext:focus()
while true do
  local itcm = ovel.itcRetrieve()
  if itcm then
    local c = 16
    while itcm and c ~= 0 do
      loop( itcm )
      c = c - 1
      itcm = ovel.itcRetrieve()
    end
  end
  if ovel.isIdle() then
    ovel.sleep( 5 )
  else
    ovel.loop()
  end
end


