#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_COMPARE_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_COMPARE_HPP_

#include <defines.hpp>
#include <types.hpp>

namespace std
{

class partial_ordering;
class weak_ordering;
class strong_ordering;

namespace internal
{

template <typename T>
struct comparison_category;

template <>
struct comparison_category<partial_ordering> {
    using type  = i8;
    using order = enum : type { less = -1, equivalent = 0, greater = 1, unordered = 2 };
};

template <>
struct comparison_category<weak_ordering> {
    using type  = i8;
    using order = enum : type { less = -1, equivalent = 0, greater = 1 };
};

template <>
struct comparison_category<strong_ordering> {
    using type  = i8;
    using order = enum : type { less = -1, equivalent = 0, greater = 1 };
};

struct unspecified {
    consteval unspecified(unspecified *) noexcept {}
};

}  // namespace internal

// ------------------------------
// std::partial_ordering
// ------------------------------

class partial_ordering
{
    // -------------------------------
    // Private types
    // -------------------------------
    using value_type  = internal::comparison_category<partial_ordering>::type;
    using unspecified = internal::unspecified;

    // -------------------------------
    // Private data members
    // -------------------------------
    value_type value;

    // -------------------------------
    // Private constructor
    // -------------------------------
    explicit constexpr partial_ordering(value_type v) noexcept : value(v) {}

    // -------------------------------
    // Friend classes
    // -------------------------------
    friend class weak_ordering;
    friend class strong_ordering;

    public:
    // ------------------------------
    // Constants
    // ------------------------------
    static const partial_ordering less;
    static const partial_ordering equivalent;
    static const partial_ordering greater;
    static const partial_ordering unordered;

    // ------------------------------
    // Comparison operators
    // ------------------------------

    NODISCARD friend constexpr bool operator==(partial_ordering o, unspecified) noexcept
    {
        return o.value == 0;
    }
    NODISCARD friend constexpr bool operator==(
        partial_ordering lhs, partial_ordering rhs
    ) noexcept = default;

    NODISCARD friend constexpr bool operator<(partial_ordering o, unspecified) noexcept
    {
        return o.value == -1;
    }
    NODISCARD friend constexpr bool operator<(unspecified, partial_ordering o) noexcept
    {
        return o.value == 1;
    }

    NODISCARD friend constexpr bool operator<=(partial_ordering o, unspecified) noexcept
    {
        return o.value <= 0;
    }
    NODISCARD friend constexpr bool operator<=(unspecified, partial_ordering o) noexcept
    {
        return static_cast<value_type>(o.value & 1) == o.value;  // 0 or 1
    }

    NODISCARD friend constexpr bool operator>(partial_ordering o, unspecified) noexcept
    {
        return o.value == 1;
    }
    NODISCARD friend constexpr bool operator>(unspecified, partial_ordering o) noexcept
    {
        return o.value == -1;
    }

    NODISCARD friend constexpr bool operator>=(partial_ordering o, unspecified) noexcept
    {
        return static_cast<value_type>(o.value & 1) == o.value;  // 0 or 1
    }
    NODISCARD friend constexpr bool operator>=(unspecified, partial_ordering o) noexcept
    {
        return o.value <= 0;
    }

    NODISCARD friend constexpr partial_ordering operator<=>(
        partial_ordering o, unspecified
    ) noexcept
    {
        return o;
    }
    NODISCARD friend constexpr partial_ordering operator<=>(
        unspecified, partial_ordering o
    ) noexcept
    {
        if (o.value & 1) {
            return partial_ordering(static_cast<value_type>(-o.value));
        } else {
            return o;
        }
    }
};

constexpr partial_ordering partial_ordering::less(
    internal::comparison_category<partial_ordering>::order::less
);
constexpr partial_ordering partial_ordering::equivalent(
    internal::comparison_category<partial_ordering>::order::equivalent
);
constexpr partial_ordering partial_ordering::greater(
    internal::comparison_category<partial_ordering>::order::greater
);
constexpr partial_ordering partial_ordering::unordered(
    internal::comparison_category<partial_ordering>::order::unordered
);

// ------------------------------
// std::weak_ordering
// ------------------------------

class weak_ordering
{
    // -------------------------------
    // Private types
    // -------------------------------
    using value_type  = internal::comparison_category<weak_ordering>::type;
    using unspecified = internal::unspecified;

    // -------------------------------
    // Private data members
    // -------------------------------
    value_type value;

    // -------------------------------
    // Private constructor
    // -------------------------------
    explicit constexpr weak_ordering(value_type v) noexcept : value(v) {}

    // -------------------------------
    // Friend classes
    // -------------------------------
    friend class strong_ordering;

    public:
    // ------------------------------
    // Constants
    // ------------------------------
    static const weak_ordering less;
    static const weak_ordering equivalent;
    static const weak_ordering greater;

    NODISCARD constexpr operator partial_ordering() const noexcept
    {
        return partial_ordering(value);
    }

    // ------------------------------
    // Comparison operators
    // ------------------------------

    NODISCARD friend constexpr bool operator==(weak_ordering o, unspecified) noexcept
    {
        return o.value == 0;
    }
    NODISCARD friend constexpr bool operator==(weak_ordering lhs, weak_ordering rhs) noexcept =
        default;

    NODISCARD friend constexpr bool operator<(weak_ordering o, unspecified) noexcept
    {
        return o.value < 0;
    }
    NODISCARD friend constexpr bool operator<(unspecified, weak_ordering o) noexcept
    {
        return o.value > 0;
    }

    NODISCARD friend constexpr bool operator<=(weak_ordering o, unspecified) noexcept
    {
        return o.value <= 0;
    }
    NODISCARD friend constexpr bool operator<=(unspecified, weak_ordering o) noexcept
    {
        return o.value >= 0;
    }

    NODISCARD friend constexpr bool operator>(weak_ordering o, unspecified) noexcept
    {
        return o.value > 0;
    }
    NODISCARD friend constexpr bool operator>(unspecified, weak_ordering o) noexcept
    {
        return o.value < 0;
    }

    NODISCARD friend constexpr bool operator>=(weak_ordering o, unspecified) noexcept
    {
        return o.value >= 0;
    }
    NODISCARD friend constexpr bool operator>=(unspecified, weak_ordering o) noexcept
    {
        return o.value <= 0;
    }

    NODISCARD friend constexpr weak_ordering operator<=>(weak_ordering o, unspecified) noexcept
    {
        return o;
    }
    NODISCARD friend constexpr weak_ordering operator<=>(unspecified, weak_ordering o) noexcept
    {
        return weak_ordering(static_cast<value_type>(-o.value));
    }
};

constexpr weak_ordering weak_ordering::less(
    internal::comparison_category<weak_ordering>::order::less
);
constexpr weak_ordering weak_ordering::equivalent(
    internal::comparison_category<weak_ordering>::order::equivalent
);
constexpr weak_ordering weak_ordering::greater(
    internal::comparison_category<weak_ordering>::order::greater
);

// ------------------------------
// std::strong_ordering
// ------------------------------

class strong_ordering
{
    // -------------------------------
    // Private types
    // -------------------------------
    using value_type  = internal::comparison_category<strong_ordering>::type;
    using unspecified = internal::unspecified;

    // -------------------------------
    // Private data members
    // -------------------------------
    value_type value;

    // -------------------------------
    // Private constructor
    // -------------------------------
    explicit constexpr strong_ordering(value_type v) noexcept : value(v) {}

    public:
    // ------------------------------
    // Constants
    // ------------------------------
    static const strong_ordering less;
    static const strong_ordering equivalent;
    static const strong_ordering equal;
    static const strong_ordering greater;

    // ------------------------------
    // Conversion operators
    // ------------------------------
    NODISCARD constexpr operator partial_ordering() const noexcept
    {
        return partial_ordering(value);
    }
    NODISCARD constexpr operator weak_ordering() const noexcept { return weak_ordering(value); }

    // ------------------------------
    // Comparison operators
    // ------------------------------

    NODISCARD friend constexpr bool operator==(strong_ordering o, unspecified) noexcept
    {
        return o.value == 0;
    }
    NODISCARD friend constexpr bool operator==(strong_ordering lhs, strong_ordering rhs) noexcept =
        default;

    NODISCARD friend constexpr bool operator<(strong_ordering o, unspecified) noexcept
    {
        return o.value < 0;
    }
    NODISCARD friend constexpr bool operator<(unspecified, strong_ordering o) noexcept
    {
        return o.value > 0;
    }

    NODISCARD friend constexpr bool operator<=(strong_ordering o, unspecified) noexcept
    {
        return o.value <= 0;
    }
    NODISCARD friend constexpr bool operator<=(unspecified, strong_ordering o) noexcept
    {
        return o.value >= 0;
    }

    NODISCARD friend constexpr bool operator>(strong_ordering o, unspecified) noexcept
    {
        return o.value > 0;
    }
    NODISCARD friend constexpr bool operator>(unspecified, strong_ordering o) noexcept
    {
        return o.value < 0;
    }

    NODISCARD friend constexpr bool operator>=(strong_ordering o, unspecified) noexcept
    {
        return o.value >= 0;
    }
    NODISCARD friend constexpr bool operator>=(unspecified, strong_ordering o) noexcept
    {
        return o.value <= 0;
    }

    NODISCARD friend constexpr strong_ordering operator<=>(strong_ordering o, unspecified) noexcept
    {
        return o;
    }
    NODISCARD friend constexpr strong_ordering operator<=>(unspecified, strong_ordering o) noexcept
    {
        return strong_ordering(static_cast<value_type>(-o.value));
    }
};

constexpr strong_ordering strong_ordering::less(
    internal::comparison_category<strong_ordering>::order::less
);
constexpr strong_ordering strong_ordering::equivalent(
    internal::comparison_category<strong_ordering>::order::equivalent
);
constexpr strong_ordering strong_ordering::equal(
    internal::comparison_category<strong_ordering>::order::equivalent
);
constexpr strong_ordering strong_ordering::greater(
    internal::comparison_category<strong_ordering>::order::greater
);

// ------------------------------
// Named comparison functions
// ------------------------------

NODISCARD constexpr bool is_eq(partial_ordering order) noexcept { return order == 0; }
NODISCARD constexpr bool is_neq(partial_ordering order) noexcept { return order != 0; }
NODISCARD constexpr bool is_lt(partial_ordering order) noexcept { return order < 0; }
NODISCARD constexpr bool is_lteq(partial_ordering order) noexcept { return order <= 0; }
NODISCARD constexpr bool is_gt(partial_ordering order) noexcept { return order > 0; }
NODISCARD constexpr bool is_gteq(partial_ordering order) noexcept { return order >= 0; }

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_COMPARE_HPP_
