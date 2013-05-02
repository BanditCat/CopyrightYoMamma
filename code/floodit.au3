AutoItSetOption("MustDeclareVars", 1) 

Dim $colors[ 6 ] = [0xed70a1, 0x605ca8, 0xf3f61d, 0xdc4a20, 0x46b1e2, 0x7e9d1e]
Dim $buttonPoss[ 9 ][ 2 ] = [ [ 576, 554 ], [ 623, 554 ], [ 668, 554 ], [ 576, 598 ], [ 623, 598 ], [ 668, 598 ], [ 593, 720 ], [ 880, 715 ], [ 880, 698 ] ]
Dim $board[ 14 ][ 14 ]
Dim $nboard[ 14 ][ 14 ]
Dim $pboard[ 14 ][ 14 ]
Dim $mx[ 4 ] = [ -1,  0, 1, 0 ]
Dim $my[ 4 ] = [  0, -1, 0, 1 ]
	
Func play( $c )
	Dim $stk[ 196 ][ 2 ], $nx, $ny, $tp, $cur, $cc, $d
	$nx = 0
	$ny = 0
	$tp = 1
	$cur = 0
	$stk[ 0 ][ 0 ] = 0
	$stk[ 0 ][ 1 ] = 0
	$cc = $nboard[ 0 ][ 0 ]
	if $cc = $c Then
		Return
	endif
	$nboard[ 0 ][ 0 ] = $c
    while $tp > $cur
		for $d = 0 to 3
			$nx = $stk[ $cur ][ 0 ] + $mx[ $d ]
			$ny = $stk[ $cur ][ 1 ] + $my[ $d ]
			if $nx >= 0 and $nx <= 13 and $ny >= 0 and $ny <= 13 Then
				if $nboard[ $nx ][ $ny ] = $cc then
					$nboard[ $nx ][ $ny ] = $c
					$stk[ $tp ][ 0 ] = $nx
					$stk[ $tp ][ 1 ] = $ny
					$tp += 1
				endif
			EndIf
		Next
		$cur += 1
	wend
EndFunc

Func score()
	Dim $stk[ 196 ][ 2 ], $nx, $ny, $tp, $cur, $cc, $d, $brdr
	Dim $ans[ 2 ]
	$brdr = 0
	$nx = 0
	$ny = 0
	$tp = 1
	$cur = 0
	$stk[ 0 ][ 0 ] = 0
	$stk[ 0 ][ 1 ] = 0
	$cc = $nboard[ 0 ][ 0 ]
	$nboard[ 0 ][ 0 ] = 6
    while $tp > $cur
		for $d = 0 to 3
			$nx = $stk[ $cur ][ 0 ] + $mx[ $d ]
			$ny = $stk[ $cur ][ 1 ] + $my[ $d ]
			if $nx >= 0 and $nx <= 13 and $ny >= 0 and $ny <= 13 Then
				if $nboard[ $nx ][ $ny ] = $cc then
					$nboard[ $nx ][ $ny ] = 6
					$stk[ $tp ][ 0 ] = $nx
					$stk[ $tp ][ 1 ] = $ny
					$tp += 1
				Elseif $nboard[ $nx ][ $ny ] < 6 Then
					$nboard[ $nx ][ $ny ] += 100
					$brdr += 1
				endif
			EndIf
		Next
		$cur += 1
	wend
	for $nx = 0 to 13
		for $ny = 0 to 13
			if $nboard[ $nx ][ $ny ] = 6 then
				$nboard[ $nx ][ $ny ] = $cc
			elseif $nboard[ $nx ][ $ny ] >= 100 then
				$nboard[ $nx ][ $ny ] -= 100
			endif
		Next
	Next
	$ans[ 0 ] = $tp
	$ans[ 1 ] = $brdr + $tp * 2
	return $ans
EndFunc


Func pfill( $v, $px, $py )
	Dim $stk[ 196 ][ 2 ], $nx, $ny, $tp, $cur, $cc, $done, $d
	$nx = 0
	$ny = 0
	$tp = 1
	$cur = 0
	$stk[ 0 ][ 0 ] = $px
	$stk[ 0 ][ 1 ] = $py
	$cc = $nboard[ $px ][ $py ]
	$pboard[ $px ][ $py ] = $v
	$nboard[ $px ][ $py ] = 6
	$done = true
    while $tp > $cur
		for $d = 0 to 3
			$nx = $stk[ $cur ][ 0 ] + $mx[ $d ]
			$ny = $stk[ $cur ][ 1 ] + $my[ $d ]
			if $nx >= 0 and $nx <= 13 and $ny >= 0 and $ny <= 13 Then
				if $nboard[ $nx ][ $ny ] = $cc then
					$nboard[ $nx ][ $ny ] = 6
					$pboard[ $nx ][ $ny ] = $v
					$stk[ $tp ][ 0 ] = $nx
					$stk[ $tp ][ 1 ] = $ny
					$tp = $tp + 1
				elseif $pboard[ $nx ][ $ny ] == 666 then
					$pboard[ $nx ][ $ny ] = $v + 1
					$done = false
				endif
			endif
		next
		$cur = $cur + 1
	wend
	for $nx = 0 to 13
		for $ny = 0 to 13
			if $nboard[ $nx ][ $ny ] = 6 Then
				$nboard[ $nx ][ $ny ] = $cc
			endif
		Next
	Next
	return $done
EndFunc

Func buildpaths()
    Dim $stk[ 196 ][ 2 ], $done, $cv, $tp, $cur
	for $x = 0 to 13
		for $y = 0 to 13
			$pboard[ $x ][ $y ] = 666
		Next
	Next
	$done = pfill( 0, 0, 0 )
	$cv = 1
	while not $done
		$tp = 0
		for $x = 0 to 13
			for $y = 0 to 13
				if $pboard[ $x ][ $y ] = $cv Then
					$stk[ $tp ][ 0 ] = $x
					$stk[ $tp ][ 1 ] = $y
					$tp = $tp + 1
				endif
			Next
		Next
		if $tp > 0 Then
			$cur = 0
			while $cur < $tp
				if $pboard[ $stk[ $cur ][ 0 ] ][ $stk[ $cur ][ 1 ] ] = $cv Then
					pfill( $cv, $stk[ $cur ][ 0 ], $stk[ $cur ][ 1 ] )
				EndIf
				$cur += 1
			WEnd
    		$cv += 1
        Else
			$done = true
		EndIf
	WEnd
	return $cv - 1
endfunc	

Func pathscore()
	dim $ans, $max
	$ans = 0
	$max = 0
	for $x = 0 to 13
		for $y = 0 to 13
			if $max < $pboard[ $x ][ $y ] Then
				$max = $pboard[ $x ][ $y ]
			EndIf
			$ans += $pboard[ $x ][ $y ]
		Next
	Next
	return $ans * $max
endfunc
	
		
func bestmove()
	dim $bgst, $cs, $ns, $ans, $max, $cc
	$bgst = 0
	$ans = 666
	$max = 666
	$cc = $board[ 0 ][ 0 ]
    for $mv1 = 0 to 5
		for $mv2 = 0 to 5
			if $mv1 == $cc or $mv2 == $mv1 Then
				ContinueLoop
			EndIf
			$nboard = $board
			play( $mv1 )
			$cs = score()
			if $cs[ 0 ] >= 196 then
				return $mv1
			endif
			play( $mv2 )
			$cs = score()
			$ns = buildpaths()
			if $ns < $max Then
				$max = $ns
				$bgst = $cs[ 1 ]
				$ans = $mv1
			elseif $ns == $max and $cs[ 1 ] > $bgst Then
				$bgst = $cs[ 1 ]
				$ans = $mv1
			endif
		next
	next
	return $ans
endfunc

func bestmove2()
	dim $bgst, $cs, $ns, $ans, $max, $cc
	$bgst = 0
	$ans = 666
	$max = 66666666666666666
	$cc = $board[ 0 ][ 0 ]
    for $mv1 = 0 to 5
		for $mv2 = 0 to 5
			if $mv1 == $cc or $mv2 == $mv1 Then
				ContinueLoop
			EndIf
			$nboard = $board
			play( $mv1 )
			$cs = score()
			if $cs[ 0 ] >= 196 then
				return $mv1
			endif
			play( $mv2 )
			buildpaths()
			$ns = pathscore()
			if $ns < $max Then
				$max = $ns
				$bgst = $cs[ 1 ]
				$ans = $mv1
			elseif $ns == $max and $cs[ 1 ] > $bgst Then
				$bgst = $cs[ 1 ]
				$ans = $mv1
			endif
		next
	next
	return $ans
endfunc

func bestmove3()
	dim $bgst, $cs, $ns, $ans, $max, $cc
	$bgst = 0
	$ans = 666
	$max = 66666666666666666
	$cc = $board[ 0 ][ 0 ]
    for $mv1 = 0 to 5
		if $mv1 == $cc Then
			ContinueLoop
		EndIf
		$nboard = $board
		play( $mv1 )
		$cs = score()
		if $cs[ 0 ] >= 196 then
			return $mv1
		endif
		buildpaths()
		$ns = pathscore()
		if $ns < $max Then
			$max = $ns
			$bgst = $cs[ 1 ]
			$ans = $mv1
		elseif $ns == $max and $cs[ 1 ] > $bgst Then
			$bgst = $cs[ 1 ]
			$ans = $mv1
		endif
	next
	return $ans
endfunc

Func shownboard( $arg )
	dim $msg
	$msg = $arg & @CRLF
	for $y = 0 to 13
		for $x = 0 to 13
			$msg = $msg & $nboard[ $x ][ $y ] & "  "
		Next
		$msg = $msg & @CRLF
	Next
	MsgBox( 0, "The Board", $msg )
endfunc
Func showpboard( $arg )
	dim $msg	
	$msg = $arg & @CRLF
	for $y = 0 to 13
		for $x = 0 to 13
			$msg = $msg & $pboard[ $x ][ $y ] & "  "
		Next
		$msg = $msg & @CRLF
	Next
	MsgBox( 0, "The Board", $msg )
endfunc
			

Func dobutton( $c )
	MouseClick( "left", $buttonPoss[ $c ][ 0 ], $buttonPoss[ $c ][ 1 ], 1, 0 )
EndFunc

Func readboard()
	dim $c
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
				exit
			endif
		Next
	Next
	$nboard = $board
EndFunc

for $i = 0 to 666666
    dim $nm, $now, $mvc, $trd, $mp
	$mp = 666
	while $mp > 12
		dobutton( 6 )
		sleep( 50 )
		readboard()	
		$mp = buildpaths()
	wend
	$trd = false
	$mvc = 0
	while not $trd
		$nm = bestmove3()
		$nboard = $board
		play( $nm )
		dobutton( $nm )
		$now = score()
		$board = $nboard
		if $now[ 0 ] >= 196 Then
			dobutton( 7 )
			$trd = True
		elseif $mvc >= 25 Then
			dobutton( 8 )
			$trd = True
		EndIf
	wend
	sleep( 100 )
next
 
