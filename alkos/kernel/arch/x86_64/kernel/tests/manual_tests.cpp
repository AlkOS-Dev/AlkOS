#include <arch_utils.hpp>
#include <test_module/test.hpp>

/**
 * Simply prints the default exception message.
 */
MTEST(VerifyDefaultExceptionMsg)
{
    const auto ptr = reinterpret_cast<u64 *>(~static_cast<u64>(0));
    ++(*ptr);
}
