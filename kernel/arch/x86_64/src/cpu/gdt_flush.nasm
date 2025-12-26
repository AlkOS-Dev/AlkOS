bits 64

global GdtFlush

section .text

GdtFlush:
    ; void GdtFlush(cpu::Gdtr* gdtr, u64 kernel_code_offset, u64 kernel_data_offset);
    ; Layout:
    ;  rdi = gdtr pointer
    ;  rsi = kernel code selector
    ;  rdx = kernel data selector
    ;  rax = return address

    lgdt [rdi] ; load new gdt

    ; load data segments
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx
    mov ss, dx

    ; load code segments
    push rsi
    lea rax, [rel .next] ; push next label address on the stack to return nicely after far jump
    push rax

    retfq

.next:
    ret
