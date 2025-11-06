#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_STRING_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_STRING_HPP_

// #include <basic_string.hpp>
#include <basic_string_view.hpp>

namespace std
{

// using string = basic_string<char>;
// using wstring = basic_string<wchar_t>;
// using u8string = basic_string<char8_t>;
// using u16string = basic_string<char16_t>;
// using u32string = basic_string<char32_t>;

using string_view    = basic_string_view<char>;
using wstring_view   = basic_string_view<wchar_t>;
using u8string_view  = basic_string_view<char8_t>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_STRING_HPP_
