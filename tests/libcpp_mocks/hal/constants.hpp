#ifndef TESTS_LIBCPP_MOCKS_HAL_CONSTANTS_HPP_
#define TESTS_LIBCPP_MOCKS_HAL_CONSTANTS_HPP_

#include <cstddef>

namespace hal
{
static constexpr std::size_t kCacheLineSizeBytes = 64;
}

namespace arch
{
static constexpr std::size_t kCacheLineSizeBytes = 64;
}

#endif  // TESTS_LIBCPP_MOCKS_HAL_CONSTANTS_HPP_
