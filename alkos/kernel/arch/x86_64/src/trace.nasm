extern QemuTerminalWriteString

%ifdef __USE_DEBUG_OUTPUT__

%macro trace 1
    push rdi
    mov rdi, %1
    call QemuTerminalWriteString
    pop rdi
%endmacro

%else

%macro trace 1
%endmacro

%endif
