; SPDX-License-Identifier: MIT
; Copyright (c) 2025-2026 The AlkOS Authors
; See the AUTHORS file for the full list of contributors.

; ------------------------------------------------------------
; Scheduling/Interrupts helpers
; ------------------------------------------------------------

%include "include/regs.nasm"
%include "include/thread.nasm"

_context_switch_stack_space equ 21*8
_context_switch_stack_space_ext equ 20*8; 8 bytes already reserved by caller
_rip_call_offset equ _context_switch_stack_space_ext
_rip_int_frame_offset equ _all_reg_size + 8
_cs_int_frame_offset equ _rip_int_frame_offset + 8
_flags_int_frame_offset equ _cs_int_frame_offset + 8
_sp_int_frame_offset equ _flags_int_frame_offset + 8
_ss_int_frame_offset equ _sp_int_frame_offset + 8

_kernel_code_selector equ 0x08
_kernel_data_selector equ 0x10
_user_code_selector equ 0x1B
_user_data_selector equ 0x23

_jump_userspace_stack_space equ  5*8  ; sizeof(IsrStackFrame)
