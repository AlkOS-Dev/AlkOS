#ifndef KERNEL_SRC_MACROS_HPP_
#define KERNEL_SRC_MACROS_HPP_

#include <expected.hpp>

#define UNEXPECTED_RET_IF_ERR(res) \
    if (!res)                      \
    return std::unexpected(res.error())

#endif  // KERNEL_SRC_MACROS_HPP_
