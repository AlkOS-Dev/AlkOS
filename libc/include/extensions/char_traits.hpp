#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_CHAR_TRAITS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_CHAR_TRAITS_HPP_

#include <string.h>
#include <extensions/compare.hpp>
#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>

#define __need_wint_t
#include <stddef.h>
#undef __need_wint_t

namespace std
{

struct mbstate_t {
};

namespace internal
{
template <typename CharT>
struct MemberTypes;

template <>
struct MemberTypes<char> {
    using char_type  = char;
    using int_type   = int;
    using off_type   = ptrdiff_t;
    using pos_type   = size_t;
    using state_type = mbstate_t;
    using comp_cat   = strong_ordering;
};

template <>
struct MemberTypes<wchar_t> {
    using char_type  = wchar_t;
    using int_type   = wint_t;
    using off_type   = ptrdiff_t;
    using pos_type   = size_t;
    using state_type = mbstate_t;
    using comp_cat   = strong_ordering;
};

template <>
struct MemberTypes<char8_t> {
    using char_type  = char8_t;
    using int_type   = unsigned char;
    using off_type   = ptrdiff_t;
    using pos_type   = size_t;
    using state_type = mbstate_t;
    using comp_cat   = strong_ordering;
};

template <>
struct MemberTypes<char16_t> {
    using char_type  = char16_t;
    using int_type   = uint_least16_t;
    using off_type   = ptrdiff_t;
    using pos_type   = size_t;
    using state_type = mbstate_t;
    using comp_cat   = strong_ordering;
};

template <>
struct MemberTypes<char32_t> {
    using char_type  = char32_t;
    using int_type   = uint_least32_t;
    using off_type   = ptrdiff_t;
    using pos_type   = size_t;
    using state_type = mbstate_t;
    using comp_cat   = strong_ordering;
};

}  // namespace internal

template <typename CharT>
class char_traits
{
    public:
    // ------------------------------
    // Member types
    // ------------------------------
    using char_type           = typename internal::MemberTypes<CharT>::char_type;
    using int_type            = typename internal::MemberTypes<CharT>::int_type;
    using off_type            = typename internal::MemberTypes<CharT>::off_type;
    using pos_type            = typename internal::MemberTypes<CharT>::pos_type;
    using state_type          = typename internal::MemberTypes<CharT>::state_type;
    using comparison_category = typename internal::MemberTypes<CharT>::comp_cat;

    // ------------------------------
    // Member functions
    // ------------------------------
    FAST_CALL constexpr char_type *assign(char_type *p, size_t n, char_type c) noexcept
    {
        if consteval {
            for (size_t i = 0; i < n; ++i) {
                p[i] = c;
            }
        } else {
            if constexpr (sizeof(char_type) == 1) {
                memset(p, static_cast<byte>(c), n);
            } else {
                for (size_t i = 0; i < n; ++i) {
                    p[i] = c;
                }
            }
        }
        return p;
    }

    NODISCARD FAST_CALL constexpr bool eq(char_type c, char_type d) noexcept { return c == d; }

    NODISCARD FAST_CALL constexpr bool lt(char_type c, char_type d) noexcept { return c < d; }

    FAST_CALL constexpr char_type *move(char_type *dest, const char_type *src, size_t n) noexcept
    {
        if consteval {
            if (dest < src || dest >= src + n) {
                for (size_t i = 0; i < n; ++i) {
                    dest[i] = src[i];
                }
            } else {
                for (size_t i = n; i > 0; --i) {
                    dest[i - 1] = src[i - 1];
                }
            }
        } else {
            memmove(dest, src, n * sizeof(char_type));
        }
        return dest;
    }

    FAST_CALL constexpr char_type *copy(char_type *dest, const char_type *src, size_t n) noexcept
    {
        if consteval {
            for (size_t i = 0; i < n; ++i) {
                dest[i] = src[i];
            }
        } else {
            memcpy(dest, src, n * sizeof(char_type));
        }
        return dest;
    }

    NODISCARD FAST_CALL constexpr int compare(
        const char_type *p, const char_type *q, size_t n
    ) noexcept
    {
        for (size_t i = 0; i < n; ++i) {
            if (lt(p[i], q[i])) {
                return -1;
            } else if (!eq(p[i], q[i])) {
                return 1;
            }
        }
        return 0;
    }

    NODISCARD FAST_CALL constexpr size_t length(const char_type *s) noexcept
    {
        const char_type *p = s;
        while (*p) {
            ++p;
        }
        return static_cast<size_t>(p - s);
    }

    NODISCARD FAST_CALL constexpr const char_type *find(
        const char_type *p, size_t n, const char_type &c
    ) noexcept
    {
        for (size_t i = 0; i < n; ++i) {
            if (eq(p[i], c)) {
                return &p[i];
            }
        }
        return nullptr;
    }

    NODISCARD FAST_CALL constexpr int_type to_int_type(char_type c) noexcept
    {
        return static_cast<int_type>(c);
    }

    NODISCARD FAST_CALL constexpr char_type to_char_type(int_type c) noexcept
    {
        return static_cast<char_type>(c);
    }

    NODISCARD FAST_CALL constexpr bool eq_int_type(int_type c1, int_type c2) noexcept
    {
        return c1 == c2;
    }

    NODISCARD FAST_CALL constexpr int_type eof() noexcept { return static_cast<int_type>(-1); }

    NODISCARD FAST_CALL constexpr int_type not_eof(int_type c) noexcept
    {
        return (c == eof()) ? 0 : c;
    }
};

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_CHAR_TRAITS_HPP_
