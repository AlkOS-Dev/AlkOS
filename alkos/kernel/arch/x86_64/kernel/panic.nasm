extern KernelPanic

%macro panic 1
    mov rdi, %1
    call KernelPanic
%endmacro
