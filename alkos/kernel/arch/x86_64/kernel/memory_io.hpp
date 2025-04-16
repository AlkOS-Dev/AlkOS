#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_IO_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_IO_HPP_

#include <extensions/defines.hpp>
#include <extensions/type_traits.hpp>

template <class NumT>
    requires std::is_unsigned_v<NumT>
FAST_CALL void WriteMemoryIo(void *base_address, const u32 offset, const NumT value)
{
    *static_cast<volatile NumT *>(base_address + offset) = value;
}

template <class NumT>
    requires std::is_unsigned_v<NumT>
FAST_CALL NumT ReadMemoryIo(void *base_address, const u32 offset)
{
    return *static_cast<volatile NumT *>(base_address + offset);
}

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_IO_HPP_
