; BCJBCJ
; 2009 Jon DuBois
; This is hereby placed in the public domain.

#requireadmin
#include <GUIConstants.au3>
#include <Process.au3>
#include <Misc.au3>


$ato = 0
$intv = 145
$modename = "off"


GUICreate( "AutoClicker", 200, 100 )
GUISetState(@SW_SHOW)
Opt("GUICoordMode", 0)
GUICtrlCreateLabel("Mode: ", 10, 30, 100, 30 ) 
$lblname = GUICtrlCreateLabel($modename, 100, 0, 100, 30  )
GUICtrlCreateLabel("Invterval: ", -100, 30, 100, 30 ) 
$lblint = GUICtrlCreateLabel($intv, 100, 0, 100, 30 )  
    

func tgl()
	If $ato == 2 then
	  TraySetIcon()
	  $modename = "off"
	  $ato = 0
	Elseif $ato == 0 then
      TraySetIcon( "warning" )
	  $modename = "clickauto"
	  $ato = 1
	Else
      TraySetIcon( "stop" )
	  $modename = "fullauto"
	  $ato = 2	  
  EndIf
  GUICtrlSetData( $lblname, $modename )
EndFunc
  
func incintv()
	$intv = $intv * 1.01
   GUICtrlSetData( $lblint, $intv )  
EndFunc

func decintv()
	$intv = $intv * 0.99
    GUICtrlSetData( $lblint, $intv )  
EndFunc

HotKeySet( "{NUMPADMULT}", "incintv" )
HotKeySet( "{NUMPADDIV}", "decintv" )
HotKeySet( "{NUMPADADD}", "tgl" )


$udll = DllOpen( "user32.dll" )
while 1 
if ( _IsPressed( "22", $udll ) and $ato == 1 ) or ( $ato == 2 ) then
MouseDown( "right" )
Sleep( $intv )
MouseUp( "right" )
endif
Sleep( 5 )
wend
