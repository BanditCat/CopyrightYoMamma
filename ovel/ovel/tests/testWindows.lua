local function nullcb( win, msg )
  if ovel.isIdle() then
    ovel.sleep( 5 )
  else
    ovel.loop()
  end
end