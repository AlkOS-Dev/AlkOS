#include "cpu/utils.hpp"
#include "trace_framework.hpp"

void TraceDumpedRegisters(const DumpedRegisters *regs)
{
    static constexpr const char *kRegNames[] = {"rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                                                "rbp", "rsp", "r8",  "r9",  "r10", "r11",
                                                "r12", "r13", "r14", "r15", "rip", "rflags",
                                                "cr0", "cr2", "cr3", "cr4"};

    static constexpr size_t kBuffSize = 1024;
    char buff[kBuffSize];
    size_t offset = 0;

    for (size_t i = 0; i < regs->flat.size() && offset < kBuffSize; ++i) {
        int written = snprintf(
            buff + offset, kBuffSize - offset, "%s: 0x%016llx\n", kRegNames[i],
            static_cast<unsigned long long>(regs->flat[i])
        );
        ASSERT_GT(written, 0);
        ASSERT_LT(written + offset, kBuffSize);
        offset += written;
    }

    TRACE_INFO_GENERAL("Dumped registers:\n%s", buff);
}
