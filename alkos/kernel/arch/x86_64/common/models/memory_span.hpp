#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_DATA_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_DATA_HPP_

#include <defines.hpp>
#include <extensions/types.hpp>

class PACK MemorySpan
{
    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//
    
    MemorySpan(const u64 start, const u64 end) : start{start}, end{end} {}
    MemorySpan(const MemorySpan &)            = default;
    MemorySpan(MemorySpan &&)                 = default;
    MemorySpan &operator=(const MemorySpan &) = delete;
    MemorySpan &operator=(MemorySpan &&)      = delete;

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//
    
    u64 GetSize() const
    {
        return end - start;
    }

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//
    
    const u64 start;
    const u64 end;
};

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_DATA_HPP_
