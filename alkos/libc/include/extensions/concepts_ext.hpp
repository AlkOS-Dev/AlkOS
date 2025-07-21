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

template <typename T, typename... Args>
concept OneOf = (std::is_same_v<T, Args> || ...);

template <class T>
concept IoT = (std::is_unsigned_v<T> && sizeof(T) <= 4);

template <typename From, typename To>
concept Narrowing = !requires(From f) { To{f}; };

template <typename From, typename To>
concept NonNarrowing = !Narrowing<From, To>;

template <class T>
concept LibCxxCompatibleAllocator =
    requires(T a, typename T::size_type n, typename T::value_type* ptr) {
        typename T::value_type;
        typename T::size_type;
        typename T::difference_type;
        typename T::propagate_on_container_move_assignment;
        { a.allocate(n) } -> std::same_as<typename T::value_type*>;
        { a.deallocate(ptr, n) } -> std::same_as<void>;
    };

template <class T>
concept LibCxxCompatibleMutex = requires(T m) {
    { m.lock() } -> std::same_as<void>;
    { m.unlock() } -> std::same_as<void>;
};
}  // namespace concepts_ext

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_CONCEPTS_EXT_HPP_
