#include "drivers/apic/io_apic.hpp"

IoApic::IoApic(const u8 id, const u32 address, const u32 gsi_base)
    : id_(id), address_(address), gsi_base_(gsi_base)
{
}
