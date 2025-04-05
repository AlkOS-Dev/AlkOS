#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_CONCEPTS_EXT_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_CONCEPTS_EXT_HPP_

#include <extensions/concepts.hpp>

/**
 * Custom concepts library for ALKOS
 */

namespace concepts_ext
{
template <typename T>
concept SwitchExpressionType = std::is_integral_v<T> || std::is_enum_v<T>;

template <typename F, typename... Args>
concept Callable = std::is_invocable_v<F, Args...>;

template <typename FuncT, typename ExprT, typename... Args>
concept RolledSwitchFunctor = requires(FuncT f, Args... args) {
    { f.template operator()<static_cast<ExprT>(0)>(args...) };
};
}  // namespace concepts_ext

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_CONCEPTS_EXT_HPP_
