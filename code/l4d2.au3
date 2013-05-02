; BCJBCJ
; 2009 Jon DuBois
; This is hereby placed in the public domain.

#requireadmin
#include <GUIConstants.au3>
#include <Process.au3>
#include <Misc.au3>

$rato = False
$ato = False
$hr = False
$intv = 103
$intj = 0.15
$modename = "  "
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
	if $ato Then
		$ato = False
		if $hr Then
			$hr = False
			GUICtrlSetData( $lblname, "  " )
			Send("{r up}")
		Else
			$hr = True			
			GUICtrlSetData( $lblname, "S " )
			Send("{r down}")
		EndIf
	Else
		$ato = True
		if $hr Then
			GUICtrlSetData( $lblname, "SP" )
		Else
			GUICtrlSetData( $lblname, " P" )
		EndIf
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

Func ex()
	Exit
EndFunc


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

	if $ato and _IsPressed( "22", $udll ) ) then
		MouseDown( "left" )
		Sleep( $del1 )
		MouseUp( "left" )
		Sleep( $del2 )
	Else
    	Sleep( $delt )
	EndIf

wend
