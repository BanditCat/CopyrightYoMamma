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
$xdelta = 0
$ydelta = 0

GUICreate( "AutoClicker", 200, 130 )
GUISetState(@SW_SHOW)
Opt("GUICoordMode", 0)
GUICtrlCreateLabel("Mode: ", 10, 30, 100, 30 ) 
$lblname = GUICtrlCreateLabel($modename, 100, 0, 100, 30  )
GUICtrlCreateLabel("X: ", -100, 30, 100, 30 ) 
$lblxdelta = GUICtrlCreateLabel($xdelta, 100, 0, 100, 30 )  
GUICtrlCreateLabel("Y: ", -100, 30, 100, 30 ) 
$lblydelta = GUICtrlCreateLabel($ydelta, 100, 0, 100, 30 )  
    

func tgl()
	If $ato == 2 then
	  TraySetIcon()
	  $modename = "off"
	  $ato = 0
	Elseif $ato == 0 then
      TraySetIcon( "warning" )
	  $modename = "getting delta"
	  $ato = 1
	Else
      TraySetIcon( "stop" )
	  $modename = "moving"
	  $ato = 2	  
  EndIf
  GUICtrlSetData( $lblname, $modename )
EndFunc
  
func disp()
	$intv = $intv * 1.01
   GUICtrlSetData( $lblxdelta, $xdelta )  
   GUICtrlSetData( $lblydelta, $ydelta )  
EndFunc

HotKeySet( "{NUMPADADD}", "tgl" )


$udll = DllOpen( "user32.dll" )
while 1 

if ( $ato == 1 ) then
  $xdelta = MouseGetPos( 0 ) / 100 - 2
  $ydelta = MouseGetPos( 1 ) / 100 - 2
elseif ( $ato = 2 ) Then
  MouseMove( MouseGetPos( 0 ) + $xdelta, MouseGetPos( 1 ) + $ydelta )
endif
disp()
Sleep( 5 )
wend
