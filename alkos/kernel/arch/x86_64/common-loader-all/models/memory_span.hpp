#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_DATA_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_DATA_HPP_

#include <defines.hpp>
#include <extensions/types.hpp>
#include "extensions/debug.hpp"

class MemorySpan
{
    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    MemorySpan() : start{0}, end{0} {}
    MemorySpan(const u64 start, const u64 end) : start{start}, end{end}
    {
        TRACE_INFO(
            "MemorySpan created: start=0x%llX, end=0x%llX, size=0x%llX", start, end, end - start
        );
    }
    MemorySpan(const MemorySpan &)            = default;
    MemorySpan(MemorySpan &&)                 = default;
    MemorySpan &operator=(const MemorySpan &) = default;
    MemorySpan &operator=(MemorySpan &&)      = default;

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    u64 GetSize() const { return end - start; }

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//

    u64 start;
    u64 end;
};

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_DATA_HPP_
