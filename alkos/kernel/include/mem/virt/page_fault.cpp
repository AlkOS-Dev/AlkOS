#include "interrupts/interrupt_types.hpp"

#include "hal/intr_parser.hpp"
#include "mem/virt/page_fault.hpp"

namespace Mem
{

void PageFaultHandler(intr::LitExcEntry &, hal::ExceptionData *data)
{
    ASSERT_NOT_NULL(data);

    Mem::PageFaultData pfd = hal::ParsePageFaultData(*data);
    return;
}

}  // namespace Mem
