; BCJBCJ
; 2009 Jon DuBois
; This is hereby placed in the public domain.

#requireadmin
#include <GUIConstants.au3>
#include <Process.au3>
#include <Misc.au3>

$rato = False
$ato = 0
$intv = 75
$intr = 1050
$intj = 0.05
$modename = "off"
$mddelay = 5

dim $mpos[ 2 ]

GUICreate( "acer", 200, 100 )
GUISetState(@SW_SHOW)
Opt("GUICoordMode", 0)
GUICtrlCreateLabel("Mode: ", 10, 30, 100, 30 ) 
$lblname = GUICtrlCreateLabel($modename, 100, 0, 100, 30  )
GUICtrlCreateLabel("Invterval: ", -100, 30, 100, 30 ) 
$lblint = GUICtrlCreateLabel($intv, 100, 0, 100, 30 )  
    
$recenter = false

func tgl()
	if $ato == 4 Then
		GUICtrlSetData( $lblint, $intv )
		$modename = "off"
		$ato = 0
		HotKeySet( "{PGDN}" )
	elseif $ato == 3 Then
		$modename = "fr2"
		$ato = 4
	elseIf $ato == 2 then
	  $rato = False	
	  TraySetIcon()
	  $modename = "fr"
	  HotKeySet( "{PGDN}", "fastr" )
	  $ato = 3
	  GUICtrlSetData( $lblint, $intr )
	Elseif $ato == 0 then
      TraySetIcon( "warning" )
	  $modename = "ca"
	  $ato = 1
	Else
      TraySetIcon( "stop" )
	  $modename = "fa"
	  $ato = 2	  
	  $rato = false
  EndIf
  remode()
EndFunc
  
func remode()
	if $recenter Then
		 GUICtrlSetData( $lblname, $modename & " +r" )
	else 
		 GUICtrlSetData( $lblname, $modename )
	 EndIf
EndFunc

func drwflwr()
	dim $spos[ 2 ]
	$spos = MouseGetPos()
	$rad = 50
	$dtr = 3.14159265358979 / 180
	Sleep( $mddelay )
	MouseMove( $spos[ 0 ] + $rad, $spos[ 1 ], 1 )
	Sleep( $mddelay )
	MouseDown( "Left" )
	for $i = 0 to 36
		Sleep( $mddelay )
		MouseMove( $spos[ 0 ] + $rad * cos( $i * 10 * $dtr ), $spos[ 1 ] + $rad * sin( $i * 10 * $dtr ), 1 )
	next
	Sleep( $mddelay )
	MouseUp( "Left" )	
	Sleep( $mddelay )
endfunc	
  
func toggleRecenter()  
	if $recenter Then
		$recenter = False
		remode()
	Else
		$mpos = MouseGetPos()
		$recenter = True
		remode()
	EndIf
EndFunc
  
func incintv()
	if $ato >= 3 Then
		$intr = $intr * 1.01
		GUICtrlSetData( $lblint, $intr )  
	Else
		$intv = $intv * 1.01
		GUICtrlSetData( $lblint, $intv )  
	EndIf
EndFunc

func decintv()
	if $ato >= 3 Then
		$intr = $intr * 0.99
		GUICtrlSetData( $lblint, $intr )  
	Else
		$intv = $intv * 0.99
		GUICtrlSetData( $lblint, $intv )  
	EndIf
EndFunc

func fastr()
	if $ato == 3 Then
		send( "r" )
	Else
		MouseDown( "left" )
		MouseUp( "left" )
	endif
	Sleep( $intr )
	send( "r" )
EndFunc

Func ex()
	Exit
EndFunc

HotKeySet( "{NUMPAD5}", "drwflwr" )
HotKeySet( "{NUMPAD6}", "toggleRecenter" )
HotKeySet( "{SCROLLLOCK}", "ex" )
HotKeySet( "{NUMPADMULT}", "incintv" )
HotKeySet( "{NUMPADDIV}", "decintv" )
HotKeySet( "{NUMPADADD}", "tgl" )

 
$udll = DllOpen( "user32.dll" )
ShellExecute( "C:\bin\nircmd.exe", "setprocesspriority af.exe abovenormal" )
$isprs = False
while 1 
	$del1 = ( 10 * random( 1 - $intj, 1 + $intj ) )
	$del2 = ( $intv * random( 1 - $intj, 1 + $intj ) )
	$delt = $del1 + $del2

	if $ato == 2 Then
		if _IsPressed( "22", $udll ) then
			if ( not $isprs ) Then
				$rato = not $rato
				$isprs = True
			EndIf
		Else
			$isprs = False
		EndIf		
	EndIf
	if ( $ato == 2 and $rato ) or ( $ato == 1 and _IsPressed( "22", $udll ) ) then
		Sleep( $del1 )
		Send( "w" )
		Sleep( $del2 )
	Else
    	Sleep( $delt )
	EndIf
	if $recenter Then
		mouseMove( $mpos[ 0 ], $mpos[ 1 ], 0 )
	EndIf
wend
