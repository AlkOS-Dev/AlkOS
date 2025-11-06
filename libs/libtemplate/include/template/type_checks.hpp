#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_TYPE_CHECKS_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_TYPE_CHECKS_HPP_

#include <concepts_ext.hpp>
#include <types.hpp>

namespace template_lib
{
// ------------------------------
// Type Checks
// ------------------------------

template <typename T, typename... Args>
constexpr size_t CountType()
{
    return (static_cast<size_t>(std::is_same_v<T, Args>) + ...);
}

template <typename T, typename... Args>
constexpr bool HasType()
{
    return concepts_ext::OneOf<T, Args...>;
}

template <typename T, typename... Args>
constexpr bool HasTypeOnce()
{
    return CountType<T, Args...>() == 1;
}

template <typename T, typename... Args>
constexpr bool HasDuplicateType()
{
    return CountType<T, Args...>() > 1;
}

template <typename... Args>
constexpr bool HasDuplicateTypes()
{
    return (HasDuplicateType<Args, Args...>() || ...);
}
}  // namespace template_lib
#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_TYPE_CHECKS_HPP_
