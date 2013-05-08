package.preload.ovel = function( name )
local table = table
local select = select
local rawset = rawset
local unpack = unpack
local pairs = pairs
local assert = assert
local loadstring = loadstring
local string = string
local math = math
local vl = vl
local debug = debug
local window = window
local widget = widget
local tonumber = tonumber
local error = error
local tostring = tostring
local getmetatable = getmetatable

module( name )

for k, v in pairs( vl ) do
  _M[ k ] = v
end


function color( ar, ag, ab )
  if type( ar ) == "color" and ag == nil and ab == nil then
    return color( ar.r, ar.g, ar.b )
  end
  if type( ar ) == "string" and ag == nil and ab == nil then
    local t = unserialize( ar )
    if type( t ) == "table" and ( t.r and type( t.r ) == "number" ) and ( t.g and type( t.g ) == "number" ) and ( t.b and type( t.b ) == "number" ) then 
      return color( t.r, t.g, t.b )
    else
      error( "Bad attempt to unserialize a color", 2 )
    end
  end
  local ans, colormeta, r, g, b = {}, { ["__type"] = "color" }, ar or 0, ag or 0, ab or 0
  function colormeta:__tostring()
    return "color{ red: " .. r .. ", green: " .. g .. ", blue: " .. b .. " }"
  end
  setmetatable( ans, colormeta )
  if ar and type( ar ) ~= "number" then error( "Bad red value passed to ovel.color", 2 ) end
  if ag and type( ag ) ~= "number" then error( "Bad green value passed to ovel.color", 2 ) end
  if ab and type( ab ) ~= "number" then error( "Bad blue value passed to ovel.color", 2 ) end
  function colormeta:__index( index )
    if index == "saturated" then return function() 
      return color( ( r < 0 ) and 0 or ( ( r > 1 ) and 1 or r ), ( g < 0 ) and 0 or ( ( g > 1 ) and 1 or g ), ( b < 0 ) and 0 or ( ( b > 1 ) and 1 or b ) ) 
    end
    elseif index == "r" or index == "red" then return r
    elseif index == "g" or index == "green" then return g 
    elseif index == "b" or index == "blue" then return b
    elseif index == "serialize" then 
      return function( self )
        local t = {}
        t.r = self.r;t.g = self.g;t.b = self.b
        return serialize( t )
      end
    else
      error( "Attempt to access bad color index " .. index .. ".", 2 )
    end
  end
  function colormeta:__newindex( index, value )
    if type( value ) ~= "number" then error( "Attempt to set a color component to a non-number.", 2 ) end
    if index == "r" or index == "red" then r = value 
    elseif index == "g" or index == "green" then g = value
    elseif index == "b" or index == "blue" then b = value 
    else
      error( "Attempt to set bad color index " .. index .. ".", 2 )
    end
  end
  return ans
end

function fontStyle( asize, acolor, abold, aitalic, aunderline, afont, amargin, abullet )
  if type( asize ) == "fontStyle" and acolor == nil and abold == nil and aitalic == nil and aunderline == nil and afont == nil and amargin == nil and abullet == nil then
    return fontStyle( asize.size, asize.color, asize.bold, asize.italic, asize.underline, asize.font, asize.margin, asize.bullet )
  end  
  if type( asize ) == "string" and acolor == nil and abold == nil and aitalic == nil and aunderline == nil and afont == nil and amargin == nil and abullet == nil then
    local t = unserialize( asize )
    if type( t ) == "table" and type( t.r ) == "number" and type( t.g ) == "number" and type( t.b ) == "number" and type( t.size ) == "number" and type( t.bold ) == "boolean" and type( t.italic ) == "boolean" and type( t.underline ) == "boolean" and type( t.font ) == "string" and ( t.margin and type( t.margin ) == "table" and type( t.margin.left ) == "number" and type( t.margin.right ) == "number" and type( t.margin.firstLine ) == "number" ) and type( t.bullet ) == "string" then 
      return fontStyle( t.size, color( t.r, t.g, t.b ), t.bold, t.italic, t.underline, t.font, t.margin, t.bullet )
    else
      error( "Bad attempt to unserialize a fontStyle", 2 )
    end
  end  
  if amargin and type( amargin ) ~= "table" then error( "Bad margin value passed to ovel.fontStyle", 2 ) end
  if not amargin then amargin = {} end
  if asize and type( asize ) ~= "number" then error( "Bad size value passed to ovel.fontStyle", 2 ) end
  if abullet and type( abullet ) ~= "string" then error( "Bad bullet value passed to ovel.fontStyle", 2 ) end  
  if acolor and type( acolor ) ~= "color" then error( "Bad color value passed to ovel.fontStyle", 2 ) end
  if afont and type( afont ) ~= "string" then error( "Bad font value passed to ovel.fontStyle", 2 ) end
  if not afont then afont = "monospace" end
  if not getFont( afont ) then error( "No matching font found for " .. afont .. ".", 2 ) end
  local ans, c, fsmeta, bold, italic, underline, size, font, firstLine, left, right, bullet = {}, color( acolor ), { ["__type"] = "fontStyle" }, abold and true or false, aitalic and true or false, aunderline and true or false, asize or 1, afont, type( amargin.firstLine ) == "number" and amargin.firstLine or 0, type( amargin.left ) == "number" and amargin.left or 0, type( amargin.right ) == "number" and amargin.right or 0, abullet or ""
  function fsmeta:__tostring()
    return "fontStyle{\n  color: " .. tostring( c ) .. "\n  font: " .. font .. "\n  size: " .. size .. "\n  bold: " .. tostring( bold ) .. "\n  italic: " .. tostring( italic ) .. "\n  underline: " .. tostring( underline ) .. "\n  first line margin: " .. tostring( firstLine ) .. "\n  left margin: " .. tostring( left ) .. "\n  right margin: " .. tostring( right ) .. "\n  bullet: " .. bullet .. "\n}\n"
  end
  setmetatable( ans, fsmeta )
  function fsmeta:__index( index )
    if index == "color" then return c 
    elseif index == "r" or index == "red" then return c.r
    elseif index == "g" or index == "green" then return c.g 
    elseif index == "b" or index == "blue" then return c.b 
    elseif index == "bold" then return bold
    elseif index == "italic" then return italic
    elseif index == "underline" then return underline
    elseif index == "size" then return size
    elseif index == "font" then return font
    elseif index == "margin" then return { ["firstLine"] = firstLine, ["left"] = left, ["right"] = right }
    elseif index == "bullet" then return bullet
    elseif index == "serialize" then 
      return function( self )
        local t = {}
        t.size = self.size;t.bold = self.bold;t.italic = self.italic;t.underline = self.underline;t.font = self.font;t.r = self.r;t.g = self.g;t.b = self.b;t.margin = self.margin;t.bullet = bullet
        return serialize( t )
      end
    else
      error( "Attempt to access bad fontstyle index " .. index .. ".", 2 )
    end
  end
  function fsmeta:__newindex( index, value )
    if index == "r" or index == "red" then c.r = value
    elseif index == "g" or index == "green" then c.g = value
    elseif index == "b" or index == "blue" then c.b = value 
    elseif index == "bold" then bold = value and true or false
    elseif index == "italic" then italic = value and true or false
    elseif index == "underline" then underline = value and true or false
    elseif index == "bullet" then bullet = value
    elseif index == "size" then 
      if type( value ) ~= "number" then error( "Attempt to set a fontStyles size to a non-number.", 2 ) end
      size = value
    elseif index == "font" then 
      if type( value ) ~= "string" then error( "Attempt to set a fontStyles font to a non-string.", 2 ) end
      if not getFont( value ) then error( "No matching font found for " .. value .. ".", 2 ) end
      font = value
    elseif index == "margin" then
      if type( value ) ~= "table" then error( "Attempt to set a fontStyles margin to a non-table.", 2 ) end
      firstLine, left, right = type( value.firstLine ) == "number" and value.firstLine or 0, type( value.left ) == "number" and value.left or 0, type( value.right ) == "number" and value.right or 0
    else
      error( "Attempt to set bad fontStyle index " .. index .. ".", 2 )
    end
  end
  return ans
end

defaultFontStyle = fontStyle()
monospaceFontStyle = fontStyle( defaultFontStyle )
serifFontStyle = fontStyle( defaultFontStyle )
sansserifFontStyle = fontStyle( defaultFontStyle )
monospaceFontStyle.font = "monospace"
serifFontStyle.font = "serif"
sansserifFontStyle.font = "sans-serif"
informationFontStyle = fontStyle( sansserifFontStyle )
informationFontStyle.size = 1.1
informationFontStyle.bold = true
warningFontStyle = fontStyle( sansserifFontStyle )
warningFontStyle.r = 1
warningFontStyle.size = 1.5
warningFontStyle.bold = true
warningFontStyle.underline = true



do
local tns = {}
local counter = 1
function tablename( t ) 
  if type( t ) ~= "table" then error( "tableName called with a non table!", 2 ) end
  if not tns[ t ] then
    tns[ t ] = "Table" .. counter
    counter = counter + 1
  end
  return tns[ t ]
end
end


processGlobals = {}
setmetatable( processGlobals, {
  __metatable = {},
  __index = function( self, key )
    if type( key ) ~= "string" and type( key ) ~= "number" then error( "Can only use literals (strings and numbers) to index processGlobals.", 2 ) end
    return getGlobal( key )
  end,
  __newindex = function( self, key, value )
    if type( key ) ~= "string" and type( key ) ~= "number" then error( "Can only use literals (strings and numbers) to index processGlobals.", 2 ) end
    if type( value ) ~= "nil" and type( value ) ~= "string" and type( value ) ~= "number" and type( value ) ~= "userdata" then error( "Can only store literals (strings, numbers and nil) in processGlobals.", 2 ) end
    setGlobal( key, value )
  end
} )
userData = {}
setmetatable( userData, {
  __metatable = {},
  __index = function( self, key )
    if type( key ) ~= "string" then error( "Can only use strings to index userData.", 2 ) end
    return getUserData( key )
  end,
  __newindex = function( self, key, value )
    if type( key ) ~= "string" then error( "Can only use strings to index userData.", 2 ) end
    if type( value ) ~= "string" then error( "Can only store strings in userData.", 2 ) end
    setUserData( key, value )
  end
} )

function getCommandLineArgs()
  local ans = {}
  local c = getGlobal"commandLineArgumentCount"
  local s = c
  if c < 2 then 
    return ans
  end
  local sw = getGlobal"commandLineArgument1"
  if sw == "-e" or sw == "/e" or sw == "-f" or sw == "/f" then
    s = 3
  else
    s = 1
  end
  while s < c do
    ans[ #ans + 1 ] = getGlobal( "commandLineArgument" .. s )
    s = s + 1
  end
  return ans
end

do
local rec = {}
function recursiveTostring( t, prefix ) 
  prefix = ( type( prefix ) == "string" and prefix ) or ""
  local ans = "\n" .. prefix
  if type( t ) == "table" then
    ans = ans .. tablename( t ) .. "{"
    rec[ t ] = true
    for k, v in pairs( t ) do
      local c = prefix .. "  " .. ( ( rec[ k ] and "<recurse " .. tablename( k ) .. ">" ) or ( ( type( k ) == "table" and tablename( k ) ) or tostring( k ) ) ) .. ": "
      ans = ans .. ( ( rec[ v ] and "\n" .. c .. "<recurse " .. tablename( v ) .. ">" ) or recursiveTostring( v, c ) )
    end
    rec[ t ] = nil
    ans = ans .. "\n" .. prefix .. "}"
  else
    ans = ans .. tostring( t )
  end
  return ans
end
end

do
local rec = {}
function recursiveCompare( t1, t2 ) 
  if t1 == t2 or rec[ t1 ] then
    return true
  end
  if type( t1 ) ~= type( t2 ) or ( type( t1 ) ~= "table" ) then
    return false
  end
  rec[ t1 ] = true
  for k, v in pairs( t1 ) do
    local failed = true
    for k2, v2 in pairs( t2 ) do
      if recursiveCompare( k, k2 ) and recursiveCompare( v, v2 ) then 
        failed = false
        break
      end
    end
    if failed then return false end
  end
  return true
end
end


do
local function metaSerialize( t, sfunc )
local rec = {}
local recc = 1
local function ser( t )
  local tp = type( t )
  if tp == "table" then
    if rec[ t ] ~= nil then 
      return "c" .. tostring( rec[ t ] ):upper()
    else
      rec[ t ] = recc
      recc = recc + 1
      local ans = "d"
      for k, v in pairs( t ) do
        ans = ans .. ser( k, rec ) .. ser( v, rec )
      end
      return ans .. "e"
    end
  elseif tp == "number" then
    return "a" .. tostring( t ):upper()
  elseif tp == "string" then
    return "b" .. sfunc( t )
  elseif tp == "boolean" then
    return "f" .. ( t and "1" or "0" )
  else
    error( "Attempt to serialize a datum that was not a string, number, boolean, or table.", 3 )
  end
end 
return ser( t )
end
function serialize( t )
  return metaSerialize( t, asciiEncode )
end
function serialCompress( t )
  return compress( metaSerialize( t, hexAsciiEncode ) )
end

function metaUnserialize( s, sfunc )
  if type( s ) ~= "string" then error( "Nonstring in metaUnserialize", 3 ) end  
  local rec = {}
  local function unser( s )
    local t = s:sub( 1, 1 )
    if t == "a" then 
      return tonumber( s:sub( 2 ) )
    elseif t == "b" then
      return sfunc( s:sub( 2 ) )
    elseif t == "c" then
      local n = tonumber( s:sub( 2 ) )
      if n == nil then return n end
      return rec[ n ]
    elseif t == "f" then
      local n = tonumber( s:sub( 2 ) )
      if n == 0 then 
        return false
      elseif n == 1 then
        return true
      else
        return nil
      end
    elseif t == "d" then
      local ss = s:match"^d(.*)e$"
      if ss == nil then return nil end
      local iter = {}
      do
        local str = ss
        local function iiter()
          if #str == 0 then return nil end
          local st, en = str:find"^%bde"
          if st == nil then 
            st, en = str:find"^[abcf][^abcdef]*" 
            if st == nil then return nil end
          end
          local ans = str:sub( st, en )
          str = str:sub( en + 1 )
          return ans
        end
        iter = function()
          local k = iiter()
          local v = iiter()
          if k ~= nil and v ~= nil then return k, v else return nil end
        end
      end 
      local tab = {}
      rec[ #rec + 1 ] = tab
      for k, v in iter do 
        local ku = unser( k )
        if ku == nil then return nil end
        local vu = unser( v )
        if vu == nil then return nil end
        tab[ ku ] = vu        
      end
      return tab
    else
      return nil
    end
  end
  return unser( s )
end
function unserialize( t )
  return metaUnserialize( t, asciiDecode )
end
function serialUncompress( t )
  return metaUnserialize( uncompress( t ), hexAsciiDecode )
end
end

function getWidgets()
  local ans = {}
  for k, v in pairs( debug.getregistry().widgets ) do
    if windowStatus( k ) then
      ans[ #ans + 1 ] = v.widget
    else
      widget.destroy( v.widget )
    end
  end
  setmetatable( ans, { __newindex = function() error( "Attempt to modify widget list directly.", 2 ) end } )
  return ans
end
function getWindows()
  local ans = {}
  for k, v in pairs( debug.getregistry().windows ) do
    if windowStatus( k ) then
      ans[ #ans + 1 ] = v.window
    else
      window.destroy( v.window )
    end
  end
  setmetatable( ans, { __newindex = function() error( "Attempt to modify window list directly.", 2 ) end } )
  return ans
end

function id( ... )
  return ...
end

function tablemap( tab, vfunc, kfunc )
  if type( tab ) ~= "table" then error( "Attempt to call tablemap on a non-table.", 2 ) end
  if kfunc == nil then kfunc = id end
  if type( vfunc ) ~= "function" or type( kfunc ) ~= "function" then error( "Attempt to call tablemap with a non-function.", 2 ) end
  local ans = {}
  for k, v in pairs( tab ) do
    ans[ kfunc( k, v ) ] = vfunc( v, k )
  end
  return ans
end
function tableany( tab )
  if type( tab ) ~= "table" then error( "Attempt to call tableany on a non-table.", 2 ) end
  for _, v in pairs( tab ) do
    if v then return v end
  end
  return nil
end





local function defwindowcb( win, type ) 
  if( type == "close" ) then 
    window.hide( win ) 
  end  
  return false 
end

local widmeta = { ["__type"] = "widget" }
local winmeta = { ["__type"] = "window" }

function winmeta:__tostring()
  local a = self.area
  local ca = self.childArea
  return "window{\n  title: " .. self.title .. "\n  style: " .. self.style .. "\n  visibility: " .. tostring( self.visibility ) ..
         "\n  area: " .. a.width .. "x" .. a.height .. "+" .. a.x .. "+" .. a.y ..
         "\n  childArea: " .. ca.width .. "x" .. ca.height .. "+" .. ca.x .. "+" .. ca.y ..
         "\n}\n"            
end
function widmeta:__tostring()
  local a = self.area
  return "widget{\n  style: " .. self.style .. "\n  visibility: " .. tostring( self.visibility ) ..
         "\n  area: " .. a.width .. "x" .. a.height .. "+" .. a.x .. "+" .. a.y ..
         "\n}\n"            
end

function winmeta:__index( index )
  if index == "visibility" then
    return window.visible( self )
  elseif index == "destroy" then
    return window.destroy
  elseif index == "style" then
    return window.getStyle( self )
  elseif index == "title" then
    return window.getTitle( self )
  elseif index == "addMenu" then
    return window.addMenu
  elseif index == "addMenuBreak" then
    return window.addMenuBreak
  elseif index == "raise" then
    return window.raise
  elseif index == "show" then
    return window.show
  elseif index == "hide" then
    return window.hide
  elseif index == "focus" then
    return window.focus    
  elseif index == "setMenuItemState" then
    return window.setMenuItemState
  elseif index == "removeMenu" then
    return window.removeMenu
  elseif index == "area" then
    local x, y, w, h = window.getPos( self, false );
    return { x = x, y = y, width = w, height = h }
  elseif index == "childArea" then
    local x, y, w, h = window.getPos( self, true );
    return { x = x, y = y, width = w, height = h }      
  else
    error( "Attempt to access bad window index " .. index .. ".", 2 )
  end
end

function widmeta:__index( index )
  if index == "visibility" then
    return widget.visible( self )
  elseif index == "text" then
    return widget.getText( self )
  elseif index == "style" then
    return widget.getStyle( self )
  elseif index == "append" then
    return function( _, txt, fmt )
      widget.select( self, -1, -1 )
      self:replaceSelection( txt, fmt );
      widget.scroll( self, 100 )
    end
  elseif index == "scroll" then
    return function( _, pcnt )
      widget.scroll( self, pcnt )
    end
  elseif index == "relativeScroll" then
    return function( _, pcnt )
      widget.relativeScroll( self, pcnt )
    end
  elseif index == "setParts" then
    if self.style ~= "statusbar" then error( "Attempt to set status text of a non-statusbar!", 2 ) end
    return widget.setParts
  elseif index == "setPartText" then
    if self.style ~= "statusbar" then error( "Attempt to set status text of a non-statusbar!", 2 ) end
    return widget.setPartText
  elseif index == "show" then
    return widget.show
  elseif index == "hide" then
    return widget.hide
  elseif index == "modified" then
    return widget.getModify( self )
  elseif index == "focus" then
    return widget.focus
  elseif index == "select" then
    return widget.select
  elseif index == "selectAll" then
    return function( _ ) widget.select( self, 0, -1 ) end
  elseif index == "selection" then
    local b, e = widget.getSelection( self )
    return { front = b, back = e }
  elseif index == "replaceSelection" then
    return function( _, txt, fmt )
      if type( fmt ) == "fontStyle" then 
        widget.replaceSelectionFormated( self, fmt, txt )
      else
        widget.replaceSelectionFormated( self, defaultFontStyle, txt )
      end
    end
  elseif index == "replaceAll" then
    return function( _, txt, fmt )
      if type( fmt ) == "fontStyle" then 
        widget.replaceAllFormated( self, fmt, txt )
      else
        widget.replaceAllFormated( self, defaultFontStyle, txt )
      end
    end    
  elseif index == "replaceSelectionWithHR" then
    return widget.replaceSelectionWithHR
  elseif index == "undo" then
    return widget.undo
  elseif index == "redo" then
    return widget.redo
  elseif index == "selectionText" then
    return widget.getSelectionText( self )
  elseif index == "area" then
    local x, y, w, h = widget.getPos( self )
    return { x = x, y = y, width = w, height = h }
  elseif index == "lineCount" then
    return widget.lineCount( self )
  elseif index == "lineFromChar" then
    return widget.lineFromChar
  elseif index == "charFromLine" then
    return widget.charFromLine
  else
    error( "Attempt to access bad widget index " .. index .. ".", 2 )
  end
end

function winmeta:__newindex( index, nval )
  if index == "visibility" then
    if nval then window.show( self ) else window.hide( self ) end
    return
  elseif index == "title" then
    window.setTitle( self, tostring( nval ) )
    return
  end
  local t = nil
  if index == "area" then t = false 
  elseif index == "childArea" then t = true end
  if t == nil then error( "Attempt to set a bad window index " .. index .. ".", 2 ) end
  if type( nval ) == "table" then 
    local xo, yo, wo, ho = window.getPos( self, t );
    local x = ( type( nval.x ) == "number" and nval.x ) or xo
    local y = ( type( nval.y ) == "number" and nval.y ) or yo
    local w = ( type( nval.width ) == "number" and nval.width ) or wo
    local h = ( type( nval.height ) == "number" and nval.height ) or ho
    window.setPos( self, x, y, w, h, t )
  else
    error( "Attempt to set window area to a non-table.", 2 )
  end
end

function widmeta:__newindex( index, nval )
  if index == "visibility" then
    if nval then widget.show( self ) else widget.hide( self ) end
    return
  elseif index == "modified" then
    return widget.setModify( self, ( nval and true ) or false )    
  elseif index == "text" then
    if self.style == "text" or self.style == "textNowrap" then
      self:replaceAll( tostring( nval ) )
    else
      window.setTitle( self, tostring( nval ) )
    end
  elseif index == "area" then 
    if type( nval ) == "table" then 
      local xo, yo, wo, ho = widget.getPos( self );
      local x = ( type( nval.x ) == "number" and nval.x ) or xo
      local y = ( type( nval.y ) == "number" and nval.y ) or yo
      local w = ( type( nval.width ) == "number" and nval.width ) or wo
      local h = ( type( nval.height ) == "number" and nval.height ) or ho
      widget.setPos( self, x, y, w, h )
    else
      error( "Attempt to set widget area to a non-table.", 2 )
    end
  else
    error( "Attempt to set a bad widget index " .. index .. ".", 2 )
  end
end

local counter = 0
local function count() counter = counter + 1; return counter end

function newwindow( ... )
  local args = {...}
  if #args > 7 then error( "Incorrect number of arguments to newwindow (should be 7 or less).", 2 ) end 
  local sbtypes = { "number", "number", "number", "number", "string", "string", "function" }
  local dsa = sysInfoDesktop();
  local defaults = { dsa.x + ( dsa.width / 8 ), dsa.y + ( dsa.height / 8 ), dsa.width * 0.666, dsa.height * 0.666, "Window " .. count(), "app", defwindowcb }
  if tableany( tablemap( tablemap( args, function( v, k ) return type( v ) end ), function( v, k ) return sbtypes[ k ] ~= v end ) ) then error( "Bad argument types in newwindow.", 2 ) end
  args = tablemap( defaults, function( v, k ) return args[ k ] or v end )
  if args[ 6 ] ~= "app" and args[ 6 ] ~= "toolbox" and args[ 6 ] ~= "borderless" then
    error( "Unrecognized window type in newwindow.", 2 )
  end

  local ans = window.new( unpack( args ) );
  setmetatable( ans, winmeta )

  return ans
end

function newwidget( par, ... )
  local args = {...}
  if #args > 6 or par == nil then error( "Incorrect number of arguments to newwidget (should be between 1 and 7 inclusive).", 2 ) end 
  if type( par ) ~= "window" then error( "First argument to newwidget was not a window.", 2 ) end
  local sbtypes = { "number", "number", "number", "number", "string", "function" }
  local dpa = par.childArea
  local defaults = { 0, 0, dpa.width, dpa.height, "text", defwindowcb }
  if tableany( tablemap( tablemap( args, function( v, k ) return type( v ) end ), function( v, k ) return sbtypes[ k ] ~= v end ) ) then error( "Bad argument types in newwidget.", 2 ) end
  args = tablemap( defaults, function( v, k ) return args[ k ] or v end )

  local ans = widget.new( par, unpack( args ) );
  setmetatable( ans, widmeta )

  return ans
end

 


function highlightLine( ln )
  if ideThread and isThreadRunning( ideThread ) then
    itcPost( "g" .. ln, ideThread );
  end
end

function ide()
  assert( loadstring( processGlobals.ide ) )()
end

ideExec = function( exstr )
  if ideThread and isThreadRunning( ideThread ) then
    itcPost( "e" .. exstr, ideThread )
  end
end

local tinform = inform
function inform( msg, ... )
  tinform( recursiveTostring( msg ), ... )
end

local tUTF32To8 = UTF32To8
function UTF32To8( u )
  local emsg = "UTF32To8 only accepts arrays of number or a single number."
  if type( u ) == "number" then 
    return tUTF32To8( u )
  elseif type( u ) == "table" then
    local ans = ""
    for i = 1, #u do
      if type( u[ i ] ) ~= "number" then error( emsg, 2 ) end
      ans = ans .. tUTF32To8( u[ i ] )
    end    
    return ans
  else
    error( emsg, 2 )
  end
end

local tUTF32To16 = UTF32To16
function UTF32To16( u, le, bom )
  local ans = ""
  local emsg = "UTF32To16 only accepts arrays of number or a single number, in addition to optional booleans."
  if ( le and type( le ) ~= "boolean" ) or ( bom and type( bom ) ~= "boolean" ) then error( emsg, 2 ) end
  if bom then
    if le then 
      ans = "\255\254"
    else
      ans = "\254\255"   
    end 
  end
  if type( u ) == "number" then 
    return ans .. ( le and tUTF32To16( u, le ) or tUTF32To16( u ) )
  elseif type( u ) == "table" then
    for i = 1, #u do
      if type( u[ i ] ) ~= "number" then error( emsg, 2 ) end
      ans = ans .. ( le and tUTF32To16( u[ i ], le ) or tUTF32To16( u[ i ] ) )
    end    
    return ans
  else
    error( emsg, 2 )
  end  
end


UTFErrorHandler = errorPrint

function UTF8Length( str )
  return UTF8CharacterPosition( str, #str )
end

function randomize()
  math.randomseed( math.fmod( unixTime(), 1000 ) * 1000 )
  for i = 1, 232 do
    local j = math.random( 1, 1000 )
  end
end

function setPrintFontStyle( fs )
  if type( fs ) ~= "fontStyle" then error( "setPrintFontStyle accepts a single fontStyle as a argument.", 2 ) end
  itcPost( "p" .. fs:serialize(), ideThread )
end
function getPrintFontStyle()
  itcPost( "f" .. threadId, ideThread )
  local m = nil
  while m == nil do
    m = itcRetrieve()
  end
  if m:sub( 1, 14 ) == "printFontStyle" then
    return fontStyle( m:sub( 15, -1 ) )
  end
end

end

require"ovel"

print = ovel.print

debug = nil
vl = nil
window = nil
widget = nil
windowStatus = nil

assert( loadstring( ovel.processGlobals.help ) )()