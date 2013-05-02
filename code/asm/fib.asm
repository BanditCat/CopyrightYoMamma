org 100h

SECTION .text


main:
	mov eax, [fib2]
	mov ebx, eax
	add eax, [fib1]
	mov [fib1], ebx
	mov [fib2], eax
	call dumpnum
	dec dword [reps]
	jnz main


exit:
	mov ah,6
	mov dl,0ffh
	int 21h
	jz exit
	int 20h


dumpnum:
	mov ecx, digitbuf + 29
	mov [digitbuf+31], byte '$'
	mov [digitbuf+30], byte 10
	mov [digitbuf+29], byte 13

	.loop:
	mov ebx, 10
	mov edx, 0
	div ebx
	add dl, '0'
	dec ecx
	mov [ecx], dl
	cmp eax, 0
	jne .loop
	
	mov ah,9
	mov dx, cx
	int 21h
ret



SECTION .data

reps dd 40
fib1 dd 0
fib2 dd 1

SECTION .bss

foo resb 200
digitbuf resb 3200
