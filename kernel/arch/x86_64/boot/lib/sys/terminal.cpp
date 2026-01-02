#include "alkos/terminal.hpp"
#include "hw/serial/qemu.hpp"

void TerminalInit()
{
    /* Initialize VGA terminal -> when multiboot allows: TODO */
    // VgaTerminalInit();

    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        QemuTerminalInit();
    }
}
