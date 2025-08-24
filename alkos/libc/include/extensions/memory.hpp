#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_MEMORY_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_MEMORY_HPP_

namespace std
{

// ------------------------------
// std::addressof
// ------------------------------

template <class T>
T* addressof(T& arg) noexcept
{
    return __builtin_addressof(arg);
}

template <class T>
const T* addressof(const T&&) = delete;

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_MEMORY_HPP_
