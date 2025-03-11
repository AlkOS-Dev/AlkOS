section .init
global _init
_init:
    push rbp
    mov rbp, rsp
    ; gcc will nicely put the contents of crtbegin.o's .init section here.

section .fini
global _fini
_fini:
    push rbp
    mov rbp, rsp
    ; gcc will nicely put the contents of crtbegin.o's .fini section here.