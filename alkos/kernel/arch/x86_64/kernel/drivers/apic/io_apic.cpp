#include "drivers/apic/io_apic.hpp"

#include <extensions/debug.hpp>
#include <todo.hpp>

IoApic::IoApic(const u8 id, const u32 address, const u32 gsi_base)
    : physical_address_(address), gsi_base_(gsi_base), id_(id)
{
    TODO_WHEN_VMEM_WORKS
    /* TODO: Map the address first, currently identity */
    virtual_address_ = physical_address_;

    version_     = static_cast<u8>(ReadRegister(kIoApicVerReg));   /* Removes upper bits */
    num_entries_ = static_cast<u8>(ReadRegister(kIoRegWin) >> 16); /* Access bits [16, 24] */

    TRACE_INFO(
        "Got IO APIC (%lu) "
        "at address: 0x%llX, "
        "version: %hhu, "
        "number of entries: %hhu, "
        "base: 0x%llX",
        id_, virtual_address_, version_, num_entries_, gsi_base_
    );
}
