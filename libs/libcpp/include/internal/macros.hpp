#ifndef LIBS_LIBCPP_INCLUDE_INTERNAL_MACROS_HPP_
#define LIBS_LIBCPP_INCLUDE_INTERNAL_MACROS_HPP_

#define RET_UNEXPECTED_IF_ERR(res) \
    if (!res)                      \
    return std::unexpected(res.error())

#define RET_UNEXPECTED_IF(cond, err) \
    if (cond)                        \
    return std::unexpected(err)

#endif  // LIBS_LIBCPP_INCLUDE_INTERNAL_MACROS_HPP_
