org 100h

main:

mov al, 13h
int 10h

mov ax, 0a000h
mov es, ax
mov ebx, 1103515245

pushad

.loop:
  popad
  mov cx, -1

  .iloop:
    mul ebx
    add eax, 12345
    mov edi, eax
    shr edi, 11
    
    mov dl, [es:di]
    cmp di, 64000
    jb .skip
    mov dl, 1fh
    .skip: cmp dl, 10h
    jbe .iloop
    dec dl    
    
    push cx
    mov cx, (cnt-ofs) / 2
    mov esi, ofs
    .iloop2:
    
      sub di, [esi]
      add esi, 2
      mov [es:di], dl
      
    loop .iloop2  
    pop cx

  loop .iloop
 

  pushad
  in al,60h
  dec al
jnz .loop

int 20h

section .data
ofs: dw 318, 320, 321, 319, 322, 318
cnt:

