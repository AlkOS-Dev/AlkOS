#ifndef KERNEL_SRC_IO_PORT_HPP_
#define KERNEL_SRC_IO_PORT_HPP_

#include "hal/io.hpp"

namespace Io
{

class IoPort
{
    IoPort(size_t addr) : addr_{addr} {};

    template <IoT T>
    void Write(T val) const
    {
        hal::IoWrite(addr_, val);
    }

    template <IoT T>
    T Read() const
    {
        hal::IoRead(addr_);
    }

    size_t addr_;
};

}  // namespace Io

#endif  // KERNEL_SRC_IO_PORT_HPP_
