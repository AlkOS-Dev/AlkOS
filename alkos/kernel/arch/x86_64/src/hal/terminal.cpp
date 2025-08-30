#include <terminal.hpp>
#include "drivers/serial_port_qemu/serial_qemu.hpp"

void TerminalInit()
{
    /* Initialize VGA terminal -> when multiboot allows: TODO */
    // VgaTerminalInit();

    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        QemuTerminalInit();
    }
}
