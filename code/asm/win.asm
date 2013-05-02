section .text


extern _MessageBoxA@16
extern _ExitProcess@4

global _main

_main:
 
  push 0
  push $-1
  push cap
  push 0
  call _MessageBoxA@16
 
  push 0
  call _ExitProcess@4
 

section .data

cap: db 'foo'