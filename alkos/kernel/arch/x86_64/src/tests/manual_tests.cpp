#include <test_module/test.hpp>
#include "cpu/utils.hpp"

/**
 * Simply prints the default exception message.
 */
MTEST(VerifyDefaultExceptionMsg)
{
    const auto ptr = reinterpret_cast<u64 *>(~static_cast<u64>(0));
    ++(*ptr);
}

MTEST(SimulateLongWork)
{
    static constexpr size_t kNumRetries    = 100;
    static constexpr size_t kNumIterations = 1000000;

    for (size_t i = 0; i < kNumRetries; ++i) {
    }
}
