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
    // ------------------------------
    // Module fields
    // ------------------------------

    private:
};
}  // namespace internal

using HardwareModule = TemplateLib::StaticSingleton<internal::HardwareModule>;

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_
