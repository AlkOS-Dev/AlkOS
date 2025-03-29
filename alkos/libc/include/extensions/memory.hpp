#ifndef ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_MEMORY_HPP_
#define ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_MEMORY_HPP_

namespace std
{

//------------------------------------------------------------------------------//
// addressof
//------------------------------------------------------------------------------//

#if __has_builtin(__builtin_addressof)
template <class T>
inline constexpr T* addressof(T& x) noexcept
{
    return __builtin_addressof(x);
}
#else
#error no __builtin_addressof support in the compiler
#endif

}  // namespace std

#endif  // ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_MEMORY_HPP_
