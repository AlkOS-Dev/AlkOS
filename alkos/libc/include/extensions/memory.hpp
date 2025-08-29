#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_MEMORY_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_MEMORY_HPP_

#include <extensions/type_traits.hpp>

namespace std
{

// ------------------------------
// std::addressof
// ------------------------------

template <typename T>
NODISCARD FORCE_INLINE_F constexpr T* addressof(T& arg) noexcept
{
    return __builtin_addressof(arg);
}

template <class T>
const T* addressof(const T&&) = delete;

// ------------------------------
// std::to_address
// ------------------------------

template <class T>
constexpr T* to_address(T* p) noexcept
{
    static_assert(!is_function_v<T>);
    return p;
}

template <class T>
constexpr auto to_address(const T& p) noexcept
{
    TODO_LIBCPP_COMPLIANCE
    return to_address(p.operator->());
}

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_MEMORY_HPP_
