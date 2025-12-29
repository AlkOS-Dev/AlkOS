#ifndef KERNEL_ARCH_X86_64_SRC_DRIVERS_PIT_PIT_HPP_
#define KERNEL_ARCH_X86_64_SRC_DRIVERS_PIT_PIT_HPP_

#include <defines.hpp>
#include <types.hpp>
#include "include/io.hpp"

namespace pit
{

static constexpr u32 kBaseFrequency = 1193182;

enum class Port : u16 { kChannel0 = 0x40, kChannel1 = 0x41, kChannel2 = 0x42, kCommand = 0x43 };

struct CommandRegister {
    enum class BinaryMode : u8 {
        kBinary = 0,  // 16-bit binary counter
        kBcd    = 1   // Binary Coded Decimal
    };

    enum class OperatingMode : u8 {
        kMode0 = 0,  // Interrupt on Terminal Count (One-shot)
        kMode1 = 1,  // Hardware Retriggerable One-Shot
        kMode2 = 2,  // Rate Generator (Periodic)
        kMode3 = 3,  // Square Wave Generator (Periodic)
        kMode4 = 4,  // Software Triggered Strobe
        kMode5 = 5   // Hardware Triggered Strobe
    };

    enum class AccessMode : u8 {
        kLatch      = 0,  // Latch Count Value
        kLsbOnly    = 1,  // Access LSB only
        kMsbOnly    = 2,  // Access MSB only
        kLsbThenMsb = 3   // Access LSB then MSB
    };

    enum class Channel : u8 { kChannel0 = 0, kChannel1 = 1, kChannel2 = 2, kReadBack = 3 };

    BinaryMode binary_mode : 1;
    OperatingMode operating_mode : 3;
    AccessMode access_mode : 2;
    Channel channel : 2;
};
static_assert(sizeof(CommandRegister) == sizeof(u8));

FAST_CALL void WriteCommand(CommandRegister reg)
{
    io::out(static_cast<u16>(Port::kCommand), *reinterpret_cast<u8 *>(&reg));
}

FAST_CALL void Disable()
{
    CommandRegister reg{};
    reg.access_mode    = CommandRegister::AccessMode::kLsbThenMsb;
    reg.operating_mode = CommandRegister::OperatingMode::kMode0;
    reg.channel        = CommandRegister::Channel::kChannel0;
    reg.binary_mode    = CommandRegister::BinaryMode::kBinary;

    WriteCommand(reg);
    io::out(static_cast<u16>(Port::kChannel0), static_cast<u8>(0));
    io::out(static_cast<u16>(Port::kChannel0), static_cast<u8>(0));
}

}  // namespace pit

#endif  // KERNEL_ARCH_X86_64_SRC_DRIVERS_PIT_PIT_HPP_
