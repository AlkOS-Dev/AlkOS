#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_PCI_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_PCI_HPP_

#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <extensions/types.hpp>
#include <io.hpp>

namespace pci
{

static constexpr u64 kPCIConfigAddressPort = 0xCF8;
static constexpr u64 kPCIConfigDataPort    = 0xCFC;

template <typename T>
    requires std::is_unsigned_v<T> && sizeof(T) <= 4
T read(u32 bus, u32 device, u32 function, u32 offset)
{
    R_ASSERT(bus < 256 && device < 32 && function < 8);
    const u32 address =
        AlignDown((bus << 16) | (device << 11) | (function << 8) | offset | kMsb<u32>, 4);
    outl(address, kPCIConfigAddressPort);
    switch (sizeof(T)) {
        case 1:
            return inb(kPCIConfigDataPort + (offset & 3));
        case 2:
            return inw(kPCIConfigDataPort + (offset & 2));
        case 4:
            return inl(kPCIConfigDataPort);
        default:
            return 0;
    }
}

template <typename T>
    requires std::is_unsigned_v<T> && (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4)
void write(u32 bus, u32 device, u32 function, u32 offset, T value)
{
    R_ASSERT(bus < 256 && device < 32 && function < 8);
    const u32 address =
        AlignDown((bus << 16) | (device << 11) | (function << 8) | offset | kMsb<u32>, 4);
    outl(address, kPCIConfigAddressPort);
    switch (sizeof(T)) {
        case 1:
            outb(kPCIConfigDataPort + (offset & 3), value);
            break;
        case 2:
            outw(kPCIConfigDataPort + (offset & 2), value);
            break;
        case 4:
            outl(kPCIConfigDataPort, value);
            break;
    }
}

}  // namespace pci

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_PCI_HPP_
