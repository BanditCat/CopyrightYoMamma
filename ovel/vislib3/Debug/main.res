        ��  ��                  J!  $   T X T   ���    0 	        dopre = [=====[

function vl.rectostring( t, prefix ) 
  prefix = ( type( prefix ) == "string" and prefix ) or ""
  ans = "\r\n" .. prefix
  if type( t ) == "table" then
    ans = ans .. "table{"
    for k, v in pairs( t ) do
      ans = ans .. vl.rectostring( v, prefix .. "  " .. tostring( k ) .. ": " )
    end
    ans = ans .. "\r\n" .. prefix .. "}"
  else
    ans = ans .. tostring( t )
  end
  return ans
end


setmetatable = vl.setmetatable
print = vl.print
type = vl.type

setmetatable( windows, { __newindex = function( s, k, v ) 
  setmetatable( v, { __mode = "kv" } )
  rawset( s, k, v )
end } )
-- BUGBUG fix
--setmetatable( widgets, { __mode = "kv" } )

function id( ... )
  return ...
end

function table.map( tab, vfunc, kfunc )
  if type( tab ) ~= "table" then error"Attempt to call table.map on a non-table." end
  if kfunc == nil then kfunc = id end
  if type( vfunc ) ~= "function" or type( kfunc ) ~= "function" then error"Attempt to call table.map with a non-function." end
  local ans = {}
  for k, v in pairs( tab ) do
    ans[ kfunc( k, v ) ] = vfunc( v, k )
  end
  return ans
end
function table.any( tab )
  if type( tab ) ~= "table" then error"Attempt to call table.any on a non-table." end
  for _, v in pairs( tab ) do
    if v then return v end
  end
  return nil
end



do

local function defwindowcb( win, type ) 
  if( type == "close" ) then window.hide( win ) end  
end

local widmeta = {}
local winmeta = {}

function winmeta:__gc()
  window.collect( self )
end
function widmeta:__gc()
  widget.collect( self )
end

function winmeta:__index( index )
  if index == "visibility" then
    return window.visible( self )
  elseif index == "title" then
    return window.getTitle( self )
  elseif index == "addMenu" then
    return window.addMenu
  elseif index == "raise" then
    return function() window.raise( self ) end
  elseif index == "show" then
    return function() window.show( self ) end
  elseif index == "hide" then
    return function() window.hide( self ) end
  elseif index == "area" then
    local x, y, w, h = window.getPos( self, false );
    return { x = x, y = y, width = w, height = h }
  elseif index == "childArea" then
    local x, y, w, h = window.getPos( self, true );
    return { x = x, y = y, width = w, height = h }      
  end 
end

function widmeta:__index( index )
  if index == "visibility" then
    return widget.visible( self )
  elseif index == "text" then
    return widget.getText( self )
  elseif index == "show" then
    return function() widget.show( self ) end
  elseif index == "hide" then
    return function() widget.hide( self ) end
  elseif index == "area" then
    local x, y, w, h = widget.getPos( self );
    return { x = x, y = y, width = w, height = h }
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
  if t == nil then error "Attempt to set a bad window index" end
  if type( nval ) == "table" then 
    local xo, yo, wo, ho = window.getPos( self, t );
    local x = ( type( nval.x ) == "number" and nval.x ) or xo
    local y = ( type( nval.y ) == "number" and nval.y ) or yo
    local w = ( type( nval.width ) == "number" and nval.width ) or wo
    local h = ( type( nval.height ) == "number" and nval.height ) or ho
    window.setPos( self, x, y, w, h, t )
  else
    error"Attempt to set window area to a non-table"
  end
end

function widmeta:__newindex( index, nval )
  if index == "visibility" then
    if nval then widget.show( self ) else widget.hide( self ) end
    return
  elseif index == "text" then
    widget.setText( self, tostring( nval ) )
    return
  elseif index == "area" then 
    if type( nval ) == "table" then 
      local xo, yo, wo, ho = widget.getPos( self );
      local x = ( type( nval.x ) == "number" and nval.x ) or xo
      local y = ( type( nval.y ) == "number" and nval.y ) or yo
      local w = ( type( nval.width ) == "number" and nval.width ) or wo
      local h = ( type( nval.height ) == "number" and nval.height ) or ho
      widget.setPos( self, x, y, w, h )
    else
      error"Attempt to set widget area to a non-table"
    end
  else
    error"Attempt to set a bad window index"
  end
end

local counter = 0
local function count() counter = counter + 1; return counter end

function newwindow( ... )
  local args = {...}
  if #args > 7 then error"Incorrect number of arguments to newwindow (should be 7 or less)." end 
  local sbtypes = { "number", "number", "number", "number", "string", "string", "function" }
  local defaults = { 50, 50, 650, 300, "Window " .. count(), "app", defwindowcb }
  if table.any( table.map( table.map( args, type ), function( v, k ) return sbtypes[ k ] ~= v end ) ) then error"Bad argument types in newwindow." end
  args = table.map( defaults, function( v, k ) return args[ k ] or v end )
  if args[ 6 ] ~= "app" and args[ 6 ] ~= "toolbox" and args[ 6 ] ~= "bordeless" then
    error"Unrecognized window type in newwindow"
  end

  local ans = window.new( unpack( args ) );
  setmetatable( ans, winmeta )

  return ans
end

function newwidget( par, ... )
  local args = {...}
  if #args > 6 or par == nil then error"Incorrect number of arguments to newwidget (should be between 1 and 7 inclusive)." end 
  if type( par ) ~= "window" then error"First argument to newwidget was not a window." end
  local sbtypes = { "number", "number", "number", "number", "string", "function" }
  local defaults = { 0, 0, 200, 200, "text", defwindowcb }
  if table.any( table.map( table.map( args, type ), function( v, k ) return sbtypes[ k ] ~= v end ) ) then error"Bad argument types in newwidget." end
  args = table.map( defaults, function( v, k ) return args[ k ] or v end )
  if args[ 5 ] ~= "text" then
    error"Unrecognized widget type in newwidget"
  end

  local ans = widget.new( par, unpack( args ) );
  setmetatable( ans, widmeta )

  return ans
end

end 

itext = {}

function maincb( win, mtype, arg1, arg2 )
  --vl.inform( tostring( win ) .. " : " .. tostring( mtype ) )
  if ( mtype == "menu" and arg1 == 11 ) or mtype == "close" then
    vl.quit()
  end
  if mtype == "move" or mtype == "resize" or mtype == "activate" then
    local sa = secw.area
    local ma = mainw.area
    sa.x = ma.x + ma.width
    sa.y = ma.y
    sa.height = ma.height
    secw.area = sa
    secw:raise()
  end
  if mtype == "keydown" then
    if arg1.name == "Ctrl+K" then
      vl.quit()
    end
  end
  if type( itext ) == "widget" then
    itext.text = "foo: " .. mtype .. ( arg1 and ( ",  arg1: " .. vl.rectostring( arg1 ) ) or "" ) .. ( arg2 and ( ",  arg2: " .. type( arg2 ) ) or "" )
    if mtype == "resize" then
      local ta = mainw.childArea; ta.x = 0; ta.y = 0
      itext.area = ta
    end  
  end
end  

function seccb( win, mtype, arg1 )

  if mtype == "menu" then
    if arg1 == 200 then
      if widget.visible( itext ) then
        widget.hide( itext )
      else
        widget.show( itext )
      end
    elseif arg1 == 201 then
      local a = itext.area
      itext.text = itext.text .. "\r\n" .. a.x .. "  " .. a.y .. "  " .. a.width .. "  " .. a.height
      a.x = a.x + 1; a.y = a.y + 1; a.width = a.width - 2; a.height = a.height - 2;
      itext.area = a
    end
  end

end  

function inputcb( win, mtype, arg1 )
  if mtype == "keyup" or mtype == "keydown" then
    maincb( mainw, mtype, arg1 )
  end
end


mainw = newwindow( 50, 50, 700, 500, "OVEL", "app", maincb )
secw = newwindow( 50, 50, 200, 200, "sec", "toolbox", seccb )

mainw.visibility = true
secw.visibility = true

mainw:addMenu( "f&ile", 1, 0 )
mainw:addMenu( "&close\tCtrl+K", 11, 1 )
mainw:addMenu( "recent", 12, 1 )
mainw:addMenu( "foo", 121, 12 )
mainw:addMenu( "bar", 122, 12 )
mainw:addMenu( "baz", 123, 12 )

secw:addMenu( "hidei", 200, 0 )
secw:addMenu( "shrinki", 201, 0 )


itext = newwidget( mainw, 10, 10, 10, 10, "text", inputcb )
do
  local a = mainw.childArea
  a.x = 0; a.y = 0
  itext.area = a
end

while true do
  vl.loop()
  vl.wait()
end

]=====]

vl.thread( dopre )


  