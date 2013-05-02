org 100h

SECTION .text

%define digs 100


mov cx, digs * 3
mov di, fib1
rep stosb
mov [fib3], byte 1

mov ah, 6
  
main:
  dec byte [reps]
  jnz .skip2
  ret
  .skip2: 

  mov di, fib1
  mov si, fib2
  mov cx, digs
  rep movsw

  mov dl, 13
  int 21h
  mov dl, 10
  int 21h
  
  mov si, fib1
  mov di, fib3
  clc
 
  .loop:
    lodsb
    adc al, [si + digs - 1]
    jz main
    
    mov dl, al
    add dl, '0'
    cmp al, 10
    jb .skip
    sub al, 10
    sub dl, 10
    stc
    .skip:
    stosb
    int 21h
  jmp .loop
    
section .data
reps: db 10

section .bss

fib1: resb digs
fib2: resb digs
fib3: resb digs



