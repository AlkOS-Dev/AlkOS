#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_LITERALS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_LITERALS_HPP_

#include <string.hpp>

namespace std::literals
{

namespace string_view_literals
{

// Suppress warning for standard library literal operators
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"

NODISCARD constexpr string_view operator""sv(const char *str, size_t len) noexcept
{
    return string_view(str, len);
}

NODISCARD constexpr u8string_view operator""sv(const char8_t *str, size_t len) noexcept
{
    return u8string_view(str, len);
}

NODISCARD constexpr u16string_view operator""sv(const char16_t *str, size_t len) noexcept
{
    return u16string_view(str, len);
}

NODISCARD constexpr u32string_view operator""sv(const char32_t *str, size_t len) noexcept
{
    return u32string_view(str, len);
}

NODISCARD constexpr wstring_view operator""sv(const wchar_t *str, size_t len) noexcept
{
    return wstring_view(str, len);
}

#pragma GCC diagnostic pop

}  // namespace string_view_literals

}  // namespace std::literals

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_LITERALS_HPP_
