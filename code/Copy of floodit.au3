
Dim $colors[ 6 ] = [0xed70a1, 0x605ca8, 0xf3f61d, 0xdc4a20, 0x46b1e2, 0x7e9d1e]
Dim $buttonPoss[ 7 ][ 2 ] = [ [ 576, 554 ], [ 623, 554 ], [ 668, 554 ], [ 576, 598 ], [ 623, 598 ], [ 668, 598 ], [ 593, 720 ] ]
Dim $board[ 14 ][ 14 ]
Dim $nboard[ 14 ][ 14 ]

Func play( $c )
	Dim $stk[ 1960 ][ 2 ]
	$score = 0
	$tp = 1
	$cur = 0
	$stk[ 0 ][ 0 ] = 0
	$stk[ 0 ][ 1 ] = 0
	$cc = $nboard[ 0 ][ 0 ]
	if $cc = $c Then
		Return 0
	endif
	while $tp > $cur
		if $nboard[ $stk[ $cur ][ 0 ] ][ $stk[ $cur ][ 1 ] ] = $cc Then
			$score = $score + 1
			$nboard[ $stk[ $cur ][ 0 ] ][ $stk[ $cur ][ 1 ] ] = $c
			if $stk[ $cur ][ 0 ] > 0 Then
				$stk[ $tp ][ 0 ] = $stk[ $cur ][ 0 ] - 1
				$stk[ $tp ][ 1 ] = $stk[ $cur ][ 1 ]
				if $nboard[ $stk[ $tp ][ 0 ] ][ $stk[ $tp ][ 1 ] ] = $c Then
					$score = $score + 1
				EndIf
				$tp = $tp + 1
			endif
			if $stk[ $cur ][ 0 ] < 13 Then
				$stk[ $tp ][ 0 ] = $stk[ $cur ][ 0 ] + 1
				$stk[ $tp ][ 1 ] = $stk[ $cur ][ 1 ]
				if $nboard[ $stk[ $tp ][ 0 ] ][ $stk[ $tp ][ 1 ] ] = $c Then
					$score = $score + 1
				EndIf
				$tp = $tp + 1
			endif
			if $stk[ $cur ][ 1 ] > 0 Then
				$stk[ $tp ][ 0 ] = $stk[ $cur ][ 0 ]
				$stk[ $tp ][ 1 ] = $stk[ $cur ][ 1 ] - 1
				if $nboard[ $stk[ $tp ][ 0 ] ][ $stk[ $tp ][ 1 ] ] = $c Then
					$score = $score + 1
				EndIf
				$tp = $tp + 1
			endif
			if $stk[ $cur ][ 1 ] < 13 Then
				$stk[ $tp ][ 0 ] = $stk[ $cur ][ 0 ]
				$stk[ $tp ][ 1 ] = $stk[ $cur ][ 1 ] + 1
				if $nboard[ $stk[ $tp ][ 0 ] ][ $stk[ $tp ][ 1 ] ] = $c Then
					$score = $score + 1
				EndIf
				$tp = $tp + 1
			endif
		endif
		$cur = $cur + 1
	wend
	return $score
EndFunc

Func bestmove()
	$sc = 0
	$max = 0
	$best = 9
	for $m1 = 0 to 5
		for $m2 = 0 to 5
			;for $m3 = 0 to 5
				$nboard = $board
				play( $m1 )
				;play( $m2 )
				$sc = play( $m2 )
				if $sc > $max Then
					$max = $sc
					$best = $m1
				endif					
			;Next
		Next
	Next
	return $best
endfunc

Func shownboard( $arg )
	$msg = $arg & @CRLF
	for $y = 0 to 13
		for $x = 0 to 13
			$msg = $msg & $nboard[ $x ][ $y ] & "  "
		Next
		$msg = $msg & @CRLF
	Next
	MsgBox( 0, "The Board", $msg )
endfunc
			

Func newgame()
	MouseClick( "left", $buttonPoss[ 6 ][ 0 ], $buttonPoss[ 6 ][ 1 ] )
EndFunc

Func readboard()
	for $x = 0 to 13
		for $y = 0 to 13
			$c = PixelGetColor( 722 + $x * 24, 545 + $y * 24 ) 
		    $board[ $x ][ $y ] = 666
		    for $i = 0 to 5
				if $colors[ $i ] = $c then
					$board[ $x ][ $y ] = $i
				EndIf
			next 
			if $board[ $x ][ $y ] = 666 then
				msgbox( 0, "Error", "Failed to read board" )
				exit()
			endif
		Next
	Next
EndFunc

readboard()
for $q = 1 to 25
    $good = bestmove()
	$foo = "Move " & $q & "  best move is " & $good 
    $nboard = $board
    play( $good )
    $board = $nboard
    shownboard( $foo )	
next