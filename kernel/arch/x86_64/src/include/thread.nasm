; ------------------------------------------------------------
; Thread structure definition
; ------------------------------------------------------------

struc Thread
    .intrusive_data         resb 144

    .tid:                   resq 1
    .owner:                 resq 1
    .flags                  resq 1
    .state                  resq 1
    .retval                 resq 1
    .wait_queue             resq 1

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
    .padding0               resb 8

    .fs_base                resq 1
    .gs_base                resq 1
    .padding1               resb 48
    .fp_state               resb 4096
endstruc
