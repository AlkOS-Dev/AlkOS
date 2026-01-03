#ifndef KERNEL_SRC_MODULES_INPUT_HPP_
#define KERNEL_SRC_MODULES_INPUT_HPP_

#include <template_lib.hpp>
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "modules/video.hpp"
#include "trace_framework.hpp"

namespace internal
{

class InputModule : public template_lib::StaticSingletonHelper
{
    protected:
    InputModule() noexcept = default;

    public:
    static Sched::Thread *KeyboardInterruptHandler(intr::LitHwEntry &)
    {
        ::HardwareModule::Get().GetPs2Keyboard().OnInterrupt();
        return nullptr;
    }

    void RegisterHardwareInterrupts()
    {
        DEBUG_INFO_GENERAL("Registering Ps2 Keyboard Handler via InputManager");
        auto &hw = ::HardwareModule::Get();
        hw.GetInterrupts()
            .GetLit()
            .InstallInterruptHandler<intr::InterruptType::kHardwareInterrupt>(
                1, intr::HwHandler{.handler = KeyboardInterruptHandler}
            );
    }

    void RouteKey(char c)
    {
        auto &wm              = ::VideoModule::Get().GetWindowManager();
        Sched::Pid target_pid = wm.GetActiveSessionFocusedPid();

        if (target_pid.id == 0) {
            return;  // No focus
        }

        auto proc_res = ::SchedulingModule::Get().GetProcesses().GetProcess(target_pid);
        if (!proc_res) {
            DEBUG_WARN_GENERAL("InputModule: Failed to find process for PID %llu", target_pid.id);
            return;
        }

        auto *proc = *proc_res;

        // Validate process pointer to prevent crashes if the map is corrupted or returns invalid
        // ptr
        if (reinterpret_cast<uintptr_t>(proc) < 0xffff800000000000) {
            DEBUG_WARN_GENERAL(
                "InputModule: Invalid process pointer %p for PID %llu", proc, target_pid.id
            );
            return;
        }

        byte b = static_cast<byte>(c);
        (void)proc->stdin_pipe.Write(std::span<const byte>(&b, 1));
    }
};

}  // namespace internal

using InputModule = template_lib::StaticSingleton<internal::InputModule>;

#endif  // KERNEL_SRC_MODULES_INPUT_HPP_
