#include <test_module/test.hpp>
#include "cpu/utils.hpp"

/**
 * Simply prints the default exception message.
 */
MTEST(VerifyDefaultExceptionMsg)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiv-by-zero"
    [[maybe_unused]] volatile int a = 9 / 0;
#pragma GCC diagnostic pop
}
