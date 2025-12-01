#ifndef KERNEL_SRC_TYPES_HPP_
#define KERNEL_SRC_TYPES_HPP_


#include <types.hpp>

template <typename T, typename E>
using Expected = std::expected<T, E>;

template <typename E>
using Unexpected = std::unexpected<E>;

#define UNEXPECTED_RET_IF_ERR(res) \
    if (!res)                      \
    return Unexpected(res.error())

#endif /* KERNEL_SRC_TYPES_HPP_ */
