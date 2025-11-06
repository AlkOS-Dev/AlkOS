#ifndef LIBS_LIBCPP_INCLUDE_MEMORY_HPP_
#define LIBS_LIBCPP_INCLUDE_MEMORY_HPP_

#include "todo.h"
#include "type_traits.hpp"
#include "utility.hpp"

namespace std
{

// ------------------------------
// std::to_address
// ------------------------------

template <class T>
constexpr T *to_address(T *p) noexcept
{
    static_assert(!std::is_function_v<T>);
    return p;
}

template <class T>
constexpr auto to_address(const T &p) noexcept
{
    TODO_LIBCPP_COMPLIANCE
    return std::to_address(p.operator->());
}

// ------------------------------
// std::addressof
// ------------------------------

template <typename T>
NODISCARD FORCE_INLINE_F constexpr T *addressof(T &arg) noexcept
{
    return __builtin_addressof(arg);
}

template <class T>
const T *addressof(const T &&) = delete;

// ------------------------------
// std::construct_at
// ------------------------------

template <class T, class... Args>
    requires(!is_unbounded_array_v<T>) &&
            requires { ::new (static_cast<void *>(nullptr)) T(std::declval<Args>()...); }
constexpr T *construct_at(T *location, Args &&...args) noexcept(
    noexcept(::new (static_cast<void *>(nullptr)) T(std::declval<Args>()...))
)
{
    if constexpr (is_array_v<T>) {
        // LWG 3436: Support for arrays in std::construct_at
        static_assert(
            sizeof...(args) == 0,
            "std::construct_at for array types must not use any "
            "arguments to initialize the array"
        );
        return ::new (static_cast<void *>(location)) T{};
    } else {
        return ::new (static_cast<void *>(location)) T(std::forward<Args>(args)...);
    }
}

// ------------------------------
// std::destroy_at
// ------------------------------

template <typename T>
constexpr void destroy_at(T *location) noexcept
{
    if constexpr (is_array_v<T>) {
        for (auto &elem : *location) {
            std::destroy_at(std::addressof(elem));
        }
    } else {
        location->~T();
    }
}

}  // namespace std

#endif  // LIBS_LIBCPP_INCLUDE_MEMORY_HPP_
