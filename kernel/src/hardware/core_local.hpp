#ifndef KERNEL_SRC_HARDWARE_CORE_LOCAL_HPP_
#define KERNEL_SRC_HARDWARE_CORE_LOCAL_HPP_

#include "hal/constants.hpp"
#include "hal/core.hpp"
#include "scheduling/thread.hpp"

namespace hardware
{
struct alignas(hal::kCacheLineSizeBytes) CoreLocal : hal::CoreLocal {
    u8 nested_interrupts;
    u16 lid;

    Sched::Thread *thread_control_block;
};

FAST_CALL CoreLocal &GetCoreLocalData()
{
    return *static_cast<CoreLocal *>(hal::GetCoreLocalData());
}

FAST_CALL Sched::Thread *GetCurrentTCB() { return GetCoreLocalData().thread_control_block; }

FAST_CALL void SetCurrentTCB(Sched::Thread *tcb) { GetCoreLocalData().thread_control_block = tcb; }

}  // namespace hardware

extern "C" Sched::Thread *cdecl_GetCurrentTCB();
extern "C" void cdecl_SetCurrentTCB(Sched::Thread *tcb);

#endif  // KERNEL_SRC_HARDWARE_CORE_LOCAL_HPP_
