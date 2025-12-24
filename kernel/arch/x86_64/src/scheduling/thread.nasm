; ------------------------------------------------------------
; Thread structure definition
; ------------------------------------------------------------

struc Thread
    .tid:          resq 1
    .owner:        resq 1
    .next:         resq 1
    .kernel_stack: resq 1
    .user_stack:   resq 1
endstruc
