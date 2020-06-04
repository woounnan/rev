; tiny.asm
BITS 32
EXTERN _exit
GLOBAL _start
SECTION .text
_start:
              push    dword 42
              call    _exit
