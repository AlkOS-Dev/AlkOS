#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_IO_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_IO_HPP_

#include <hal/api/io.hpp>

#include <limits.hpp>

#include "include/io.hpp"

namespace arch
{

template <IoT T>
void IoWrite(size_t addr, T value)
{
    ASSERT_LE(addr, std::numeric_limits<u16>::max());
    io::out(static_cast<u16>(addr), value);
}

template <IoT T>
T IoRead(size_t addr)
{
    ASSERT_LE(addr, std::numeric_limits<u16>::max());
    return io::in<T>(static_cast<u16>(addr));
}

}  // namespace arch

#endif /* KERNEL_ARCH_X86_64_SRC_HAL_IMPL_IO_HPP_ */
