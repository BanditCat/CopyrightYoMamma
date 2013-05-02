
$pid = Run(@comspec & " /c " & chr(34) & '"C:\Users\Aristotle\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\GitHub, Inc\GitHub.appref-ms"' & chr(34) & " --open-shell")
sleep( 10000 )
WinActivate( "Powershell.exe" )
send( "cd G:\root\home\Aristotle\lnz{ENTER}" );
sleep( 2000 )
send( "git push origin master{ENTER}" );