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

#define PREPARE_CORE_LOCAL_ACCESS(name, rv, field)                       \
    FAST_CALL void SetCoreLocal##name(rv value)                          \
    {                                                                    \
        hal::SetCoreLocalField<rv, offsetof(CoreLocal, field)>(value);   \
    }                                                                    \
    NODISCARD FAST_CALL rv GetCoreLocal##name()                          \
    {                                                                    \
        return hal::GetCoreLocalField<rv, offsetof(CoreLocal, field)>(); \
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
PREPARE_CORE_LOCAL_ACCESS(NestedInterrupts, u8, nested_interrupts);
PREPARE_CORE_LOCAL_ACCESS(Lid, u16, lid);
PREPARE_CORE_LOCAL_ACCESS(Tcb, Sched::Thread *, thread_control_block);
PREPARE_CORE_LOCAL_ACCESS(Self, CoreLocal *, self);
#pragma GCC diagnostic pop

#define GET_CORE_LOCAL_THREAD_SAVE()           \
    []() {                                     \
        const auto thread = GetCoreLocalTcb(); \
        ASSERT_NOT_NULL(thread);               \
        return thread;                         \
    }()

NODISCARD FAST_CALL Sched::Pid GetRunningPid()
{
    // During boot, TCB is null. Return {0,0} (Kernel PID) in that case.
    auto *tcb = GetCoreLocalTcb();
    if (tcb == nullptr) {
        return {0, 0};
    }
    return GetCoreLocalTcb()->owner;
}

}  // namespace hardware

#endif  // KERNEL_SRC_HARDWARE_CORE_LOCAL_HPP_
