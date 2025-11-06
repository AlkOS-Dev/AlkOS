#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_INCLUDE_MEMORY_IO_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_INCLUDE_MEMORY_IO_HPP_

#include <extensions/bit.hpp>
#include <extensions/defines.hpp>
#include <extensions/type_traits.hpp>

template <class NumT>
    requires std::is_unsigned_v<NumT>
FAST_CALL void WriteMemoryIoRaw(byte *base_address, const u32 offset, const NumT value)
{
    *reinterpret_cast<volatile NumT *>(base_address + offset) = value;
}

template <class NumT>
    requires std::is_unsigned_v<NumT>
NODISCARD FAST_CALL NumT ReadMemoryIoRaw(const byte *base_address, const u32 offset)
{
    return *reinterpret_cast<const volatile NumT *>(base_address + offset);
}

template <class TargetT, class NumT>
    requires(std::is_integral_v<NumT> && sizeof(TargetT) <= sizeof(NumT))
NODISCARD TargetT CastRegister(const NumT value)
{
    return *reinterpret_cast<const TargetT *>(&value);
}

template <class TargetT>
    requires(sizeof(TargetT) <= 8)
NODISCARD auto ToRawRegister(const TargetT value)
{
    return *reinterpret_cast<const typename UnsignedIntegral<sizeof(TargetT)>::type *>(&value);
}

template <class RegSizeT, class InputT>
    requires(sizeof(InputT) <= sizeof(RegSizeT))
FAST_CALL void WriteMemoryIo(byte *base_address, const u32 offset, const InputT value)
{
    const RegSizeT casted_value = ToRawRegister(value);
    WriteMemoryIoRaw<RegSizeT>(base_address, offset, casted_value);
}

template <class RegSizeT, class RetT>
    requires(sizeof(RetT) <= sizeof(RegSizeT))
NODISCARD FAST_CALL RetT ReadMemoryIo(const byte *base_address, const u32 offset)
{
    const RegSizeT reg = ReadMemoryIoRaw<RegSizeT>(base_address, offset);
    return CastRegister<RetT>(reg);
}

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_INCLUDE_MEMORY_IO_HPP_
