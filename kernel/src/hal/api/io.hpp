#ifndef KERNEL_SRC_HAL_API_IO_HPP_
#define KERNEL_SRC_HAL_API_IO_HPP_

#include <concepts.hpp>
#include <types.hpp>

namespace arch
{

template <typename T>
concept IoT = std::is_same_v<T, u8> || std::is_same_v<T, u16> || std::is_same_v<T, u32>;

template <IoT T>
void IoWrite(size_t addr, T value);
template <IoT T>
T IoRead(size_t addr);

}  // namespace arch

namespace hal
{
using arch::IoT;
}

#endif  // KERNEL_SRC_HAL_API_IO_HPP_
