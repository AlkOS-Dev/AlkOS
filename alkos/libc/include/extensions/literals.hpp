#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_LITERALS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_LITERALS_HPP_

#include <extensions/string.hpp>

namespace std::literals
{

namespace string_view_literals
{

NODISCARD constexpr string_view operator""sv(const char* str, size_t len) noexcept
{
    return string_view(str, len);
}

NODISCARD constexpr u8string_view operator""sv(const char8_t* str, size_t len) noexcept
{
    return u8string_view(str, len);
}

NODISCARD constexpr u16string_view operator""sv(const char16_t* str, size_t len) noexcept
{
    return u16string_view(str, len);
}

NODISCARD constexpr u32string_view operator""sv(const char32_t* str, size_t len) noexcept
{
    return u32string_view(str, len);
}

NODISCARD constexpr wstring_view operator""sv(const wchar_t* str, size_t len) noexcept
{
    return wstring_view(str, len);
}

}  // namespace string_view_literals

}  // namespace std::literals

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_LITERALS_HPP_
