; ------------------------------------------------------------
; Thread structure definition
; ------------------------------------------------------------

struc Thread
    .fs_base       resq 1
    .gs_base       resq 1
    .tid:          resq 1
    .owner:        resq 1
    .next:         resq 1
    .kernel_stack: resq 1
    .kernel_stack_bottom: resq 1
    .user_stack:   resq 1
    .user_stack_bottom:   resq 1
endstruc
