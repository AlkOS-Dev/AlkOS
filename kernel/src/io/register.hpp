#ifndef KERNEL_SRC_IO_REGISTER_HPP_
#define KERNEL_SRC_IO_REGISTER_HPP_

#include "hal/io.hpp"

namespace Io
{

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

}  // namespace Io

#endif  // KERNEL_SRC_IO_REGISTER_HPP_
