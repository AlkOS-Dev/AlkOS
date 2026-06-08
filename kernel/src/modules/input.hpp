#ifndef KERNEL_SRC_MODULES_INPUT_HPP_
#define KERNEL_SRC_MODULES_INPUT_HPP_

#include <template_lib.hpp>
#include "drivers/input/virtual_key.hpp"
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
    void RouteKey(VirtualKey vk, KeyModifiers modifiers)
    {
        // Try to convert virtual key to ASCII
        auto ascii_opt = Drivers::Input::VirtualKeyToAscii(vk, modifiers);
        if (!ascii_opt) {
            return;  // Key has no ASCII representation, ignore for now
        }

        char c = *ascii_opt;

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

        byte b = static_cast<byte>(c);
        (void)proc->stdin_pipe.Write(std::span<const byte>(&b, 1));
    }
};

}  // namespace internal

using InputModule = template_lib::StaticSingleton<internal::InputModule>;

#endif  // KERNEL_SRC_MODULES_INPUT_HPP_
