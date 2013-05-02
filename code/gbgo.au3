; BCJBCJ
; 2009 Jon DuBois
; This is hereby placed in the public domain.

#requireadmin
#include <GUIConstants.au3>
#include <Process.au3>
#include <Misc.au3>

opt( "MouseCoordMode", 1 )
opt( "MouseClickDownDelay", 0 )
dim $spos[ 2 ]
dim $xvec[ 2 ]
dim $yvec[ 2 ]

$tiledim = 26

$xvec[ 0 ] = 15.087
$xvec[ 1 ] = -7.261
$yvec[ 0 ] = -15.087
$yvec[ 1 ] = -7.261

$spos[ 0 ] = 729 
$spos[ 1 ] = 766

$break = False

GUICreate( "fvgo", 200, 100 )
GUISetState(@SW_SHOW)
Opt("GUICoordMode", 0)
GUICtrlCreateLabel("Width:  ", 10, 30, 100, 30 ) 
$lblname = GUICtrlCreateLabel($xvec[ 0 ], 100, 0, 100, 30  )
GUICtrlCreateLabel("Height: ", -100, 30, 100, 30 ) 
$lblint = GUICtrlCreateLabel($yvec[ 1 ] * -1, 100, 0, 100, 30 )  
    
dim $grid[ 26 ][ 26 ] = [ [ 2, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
						  [ 2, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
						  [ 2, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
						  [ 2, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
                          [ 2, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1 ], _
                          [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
						  [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
                          [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], _
                          [ 2, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 0 ], _
                          [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 ], _
						  [ 2, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 0 ], _
                          [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 ], _
                          [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 ], _
                          [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 ] ]


func getpos()
	$spos = MouseGetPos()
EndFunc

func setpos()
	MouseMove( $spos[ 0 ], $spos[ 1 ], 1 )
endfunc

func spdfrm( $w )


	for $x = ( $tiledim - 1 ) to 0 step -1
		for $y = ( $tiledim - 1 ) to 0 step -1
			if $break Then
				$break = False
				Return
			elseif $grid[ $x ][ $y ] >= $w  Then
				sleep( 10 );
				MouseClick( "left", $spos[ 0 ] + $xvec[ 0 ] * $x + $yvec[ 0 ] * $y, $spos[ 1 ] + $xvec[ 1 ] * $x + $yvec[ 1 ] * $y, 1, 1 )	
			endif
		Next
	Next

	MouseMove( $spos[ 0 ], $spos[ 1 ], 2 )
EndFunc

func sqrfrm()
	for $r = 0 to 23 
		for $x = 0 to ( $r - 1 )
			if $break Then
				$break = False
				Return
			endif
			sleep( 10 );
			MouseClick( "left", $spos[ 0 ] + $xvec[ 0 ] * $x + $yvec[ 0 ] * $r, $spos[ 1 ] + $xvec[ 1 ] * $x + $yvec[ 1 ] * $r, 1, 1 )	
		Next
		for $y = 0 to $r
			if $break Then
				$break = False
				Return
			endif
			sleep( 10 );
			MouseClick( "left", $spos[ 0 ] + $xvec[ 0 ] * $r + $yvec[ 0 ] * $y, $spos[ 1 ] + $xvec[ 1 ] * $r + $yvec[ 1 ] * $y, 1, 1 )	
		Next
	Next
endfunc	
  
func tgl()
	spdfrm( 1 )
EndFunc

func tgl2()
	spdfrm( 2 )
EndFunc    

Func ex()
	Exit
EndFunc

func brk()
	$break = True
endfunc

; chicken stuff

$sd = 250
$ld = 750

$cxd = -85
$cyd = 50

dim $chkb[ 4 ][ 2 ] = [ [ 626, 533 ], [ 918, 533 ], [ 626, 720 ], [ 918, 720 ] ]


func gotoChcspt( $n )
	$gcpx = ( floor( $n / 2 ) * -25 ) + ( 25 * mod( $n, 2 ) )
	$gcpy = ( floor( $n / 2 ) * 12 ) + ( 12 * mod( $n, 2 ) )
	MouseMove( $spos[ 0 ] + $cxd + $gcpx, $spos[ 1 ] + $cyd + $gcpy, 1 )
endfunc	

func chko( $n )
	MouseClick( "left", $spos[ 0 ], $spos[ 1 ], 1, 1 )	
	sleep( $sd );
	MouseClick( "left", $spos[ 0 ] + 29, $spos[ 1 ] + 9, 1, 1 )	
	sleep( $ld );
	if $n > 4 Then
		MouseClick( "left", 1166, 557, 1, 1 )	
		sleep( $ld )
		$b = $n - 5
	Else
		$b = $n - 1
	EndIf
	MouseClick( "left", $chkb[ $b ][ 0 ], $chkb[ $b ][ 1 ], 1, 1 )	
	sleep( $sd );
EndFunc

func mvtl()
	MouseMove( 1100, 960, 1 )	
	sleep( $sd )
	MouseClick( "left", 1100, 830, 1, 1 )
endfunc

func mltitl()
	MouseClick( "left", 1100, 960, 1, 1 )
endfunc

$foo = 0
func dochickens()
	chko( 7 )
	gotoChcspt( 42 )
EndFunc

HotKeySet( "{NUMPAD1}", "dochickens" )
HotKeySet( "{NUMPAD2}", "mltitl" )


HotKeySet( "{PAUSE}", "brk" )
HotKeySet( "{SCROLLLOCK}", "ex" )
HotKeySet( "{NUMPADADD}", "tgl" )
HotKeySet( "{NUMPADDIV}", "tgl2" )
HotKeySet( "{NUMPADMULT}", "sqrfrm" )
HotKeySet( "{NUMPAD5}", "getpos" )
HotKeySet( "{NUMPAD6}", "setpos" )
ShellExecute( "C:\bin\nircmd.exe", "setprocesspriority gbgo.exe abovenormal" )

$udll = DllOpen( "user32.dll" )
while 1 
  	Sleep( 100 )
wend
