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
