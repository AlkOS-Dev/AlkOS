#ifndef KERNEL_SRC_MACROS_HPP_
#define KERNEL_SRC_MACROS_HPP_

#define RET_UNEXPECTED_IF_ERR(res) \
    if (!res)                      \
        return std::unexpected(res.error())

#define RET_UNEXPECTED_IF(cond, err) \
    if (cond)                        \
        return std::unexpected(err)

#endif  // KERNEL_SRC_MACROS_HPP_
