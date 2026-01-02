#ifndef KERNEL_SRC_SYS_ELF_ELF_HPP_
#define KERNEL_SRC_SYS_ELF_ELF_HPP_

#include <types.h>
#include <defines.hpp>

namespace System::Elf
{

// ELF-64 Structures based on System V ABI

struct Header {
    unsigned char identifier[16];
    u16 type;
    u16 machine;
    u32 version;
    u64 entry;
    u64 phoff;
    u64 shoff;
    u32 flags;
    u16 ehsize;
    u16 phentsize;
    u16 phnum;
    u16 shentsize;
    u16 shnum;
    u16 shstrndx;

    static constexpr unsigned char kMagic[4] = {0x7F, 'E', 'L', 'F'};
    static constexpr u16 kTypeExec           = 2;
} PACK;

struct ProgramHeader {
    u32 type;
    u32 flags;
    u64 offset;
    u64 vaddr;
    u64 paddr;
    u64 filesz;
    u64 memsz;
    u64 align;

    static constexpr u32 kTypeLoad  = 1;
    static constexpr u32 kFlagExec  = 1;
    static constexpr u32 kFlagWrite = 2;
    static constexpr u32 kFlagRead  = 4;
} PACK;

}  // namespace System::Elf

#endif  // KERNEL_SRC_SYS_ELF_ELF_HPP_
