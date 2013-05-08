local function nullcb( win, msg )  return trueendw = {}w.foo = nullcbx = 0y = 0i = 0w = {}for i = 1, 3 do  w[ i ] = ovel.newwindow( x, y, 200, 200, "test " .. i, "app", nullcb )  w[ i ]:show()  print( tostring( w[ i ] ) )  ovel.newwidget( w[ i ], 5, 5, 50, 170, "text" )  ovel.newwidget( w[ i ], 55, 5, 50, 170, "text" )  ovel.newwidget( w[ i ], 200, 200, 200, 200, "statusbar" )  i = i + 1  x = x + 30  y = y + 20endlocal c = ovel.clock()while true do  nullcb()  if ovel.clock() - c > 0.1 then     ovel.ideExec[[otext.text = ""]]    print( ovel.recursiveTostring( ovel.getWindows() ) .. "\n\n" )    print( ovel.recursiveTostring( ovel.getWidgets() ) )    c = ovel.clock()  end       
  if ovel.isIdle() then
    ovel.sleep( 5 )
  else
    ovel.loop()
  end  if #ovel.getWindows() == 0 then ovel.quit() end     
end