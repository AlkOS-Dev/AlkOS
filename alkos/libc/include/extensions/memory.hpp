#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_MEMORY_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_MEMORY_HPP_

#include <extensions/type_traits.hpp>
#include <extensions/utility.hpp>

namespace std
{

// ------------------------------
// std::construct_at
// ------------------------------

template <class T, class... Args>
constexpr T* construct_at(T* location, Args&&... args)
{
    if constexpr (std::is_array_v<T>)
        return ::new (std::addressof(*location)) T[1]();
    else
        return ::new (std::addressof(*location)) T(std::forward<Args>(args)...);
}

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_MEMORY_HPP_
