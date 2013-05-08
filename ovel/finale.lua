if ovel then
  if #ovel.getPrintBuffer() > 1 then
    ovel.inform( ovel.getPrintBuffer(), "Buffered Console Output" )
  end
  local emsg = ovel.getErrorPrintBuffer()
  if #emsg > 1 then
    local iside = ( ovel.ideThread and ovel.isThreadRunning( ovel.ideThread ) )
    _, _, cap = string.find( emsg, '%[string "<new thread>"%]:(%d+):' )
    if cap and iside then ovel.highlightLine( tonumber( cap ) ) end
    if iside or ovel.isConsoleErrorAttached() then
      ovel.errorPrint( emsg )
    else
      ovel.informError( emsg )
    end
  end 
end
