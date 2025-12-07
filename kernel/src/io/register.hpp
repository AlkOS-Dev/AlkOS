#ifndef KERNEL_SRC_IO_REGISTER_HPP_
#define KERNEL_SRC_IO_REGISTER_HPP_

#include "hal/io.hpp"

namespace IO
{

/**
 * @class Register
 * @brief Abstraction for a specific hardware I/O register or port.
 *
 * This class encapsulates the address of a hardware register (e.g., an I/O port number
 * on x86 or a physical memory address on MMIO architectures)
 *
 */
class Register
{
    public:
    explicit Register(size_t addr) : addr_{addr} {};
    Register(size_t base_addr, size_t offset) : addr_{base_addr + offset} {};

    template <hal::IoT T>
    void Write(T val) const
    {
        hal::IoWrite(addr_, val);
    }

    template <hal::IoT T>
    T Read() const
    {
        return hal::IoRead<T>(addr_);
    }

    private:
    size_t addr_;
};

}  // namespace IO

#endif  // KERNEL_SRC_IO_REGISTER_HPP_
