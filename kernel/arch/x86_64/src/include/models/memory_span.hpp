#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_INCLUDE_MODELS_MEMORY_SPAN_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_INCLUDE_MODELS_MEMORY_SPAN_HPP_

#include <extensions/types.hpp>
class MemorySpan
{
    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    MemorySpan() : start{0}, end{0} {}
    MemorySpan(u64 start, u64 end) : start{start}, end{end} {}
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

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_INCLUDE_MODELS_MEMORY_SPAN_HPP_
