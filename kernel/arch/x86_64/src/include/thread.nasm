; ------------------------------------------------------------
; Thread structure definition
; ------------------------------------------------------------

struc Thread
    .fp_state               resb 4096
    .fs_base                resq 1
    .gs_base                resq 1
    .tid:                   resq 1
    .owner:                 resq 1
    .flags                  resq 1
    .state                  resq 1
    .next:                  resq 1
    .kernel_stack:          resq 1
    .kernel_stack_bottom:   resq 1
    .user_stack:            resq 1
    .user_stack_bottom:     resq 1

    .kernel_time_ns         resq 1
    .user_time_ns           resq 1
    .timestamp              resq 1
    .num_interrupts         resq 1
    .num_syscalls           resq 1
    .num_context_switches   resq 1
endstruc
