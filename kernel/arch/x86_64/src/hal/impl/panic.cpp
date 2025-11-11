#include <autogen/feature_flags.h>
#include <test_module/test_module.hpp>

#include "hal/impl/panic.hpp"

#include "cpu/utils.hpp"
#include "trace_framework.hpp"

namespace arch
{

extern "C" void NO_RET KernelPanic()
{
    if constexpr (FeatureEnabled<FeatureFlag::kRunTestMode>) {
        test::OnKernelPanic();
    } else {
        trace::DumpAllBuffersOnFailure();
        OsHang();
    }
}

}  // namespace arch
