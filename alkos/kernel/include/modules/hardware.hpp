#ifndef ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_

#include <extensions/template_lib.hpp>
#include <hardware/interupts.hpp>

namespace internal
{
class HardwareModule : TemplateLib::StaticSingletonHelper
{
    // -------------------------------------
    // Protected singleton constructor
    // -------------------------------------

    protected:
    HardwareModule() noexcept;

    // ------------------------------
    // Getters
    // ------------------------------

    public:
    FORCE_INLINE_F hardware::Interrupts& GetInterrupts() noexcept { return interrupts_; }

    // ------------------------------
    // Module fields
    // ------------------------------

    private:
    hardware::Interrupts interrupts_;
};
}  // namespace internal

using HardwareModule = TemplateLib::StaticSingleton<internal::HardwareModule>;

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_
