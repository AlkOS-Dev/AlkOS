#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_LIMITS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_LIMITS_HPP_

#include <extensions/bits_ext.hpp>
#include <extensions/type_traits.hpp>

namespace internal
{
static constexpr int digits10(const int digits)
{
    return digits * 693147 / 2302585;  // log10(2) approximation up to 7 decimal digits
}
}  // namespace internal

namespace std
{
enum float_round_style {
    round_indeterminate       = -1,
    round_toward_zero         = 0,
    round_to_nearest          = 1,
    round_toward_infinity     = 2,
    round_toward_neg_infinity = 3
};

enum float_denorm_style { denorm_indeterminate = -1, denorm_absent = 0, denorm_present = 1 };

template <typename T>
struct numeric_limits {
    // ------------------------------
    // Constant Fields
    // ------------------------------

    static constexpr bool is_specialized           = false;
    static constexpr int digits                    = 0;
    static constexpr int digits10                  = 0;
    static constexpr int max_digits10              = 0;
    static constexpr bool is_signed                = false;
    static constexpr bool is_integer               = false;
    static constexpr bool is_exact                 = false;
    static constexpr int radix                     = 0;
    static constexpr int min_exponent              = 0;
    static constexpr int min_exponent10            = 0;
    static constexpr int max_exponent              = 0;
    static constexpr int max_exponent10            = 0;
    static constexpr bool has_infinity             = false;
    static constexpr bool has_quiet_NaN            = false;
    static constexpr bool has_signaling_NaN        = false;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss          = false;
    static constexpr bool is_iec559                = false;
    static constexpr bool is_bounded               = false;
    static constexpr bool is_modulo                = false;
    static constexpr bool traps                    = false;
    static constexpr bool tinyness_before          = false;
    static constexpr float_round_style round_style = round_toward_zero;

    // ------------------------------
    // Functions
    // ------------------------------

    static constexpr T min() noexcept { return T(); }
    static constexpr T max() noexcept { return T(); }
    static constexpr T lowest() noexcept { return T(); }
    static constexpr T epsilon() noexcept { return T(); }
    static constexpr T round_error() noexcept { return T(); }
    static constexpr T infinity() noexcept { return T(); }
    static constexpr T quiet_NaN() noexcept { return T(); }
    static constexpr T signaling_NaN() noexcept { return T(); }
    static constexpr T denorm_min() noexcept { return T(); }
};

// ------------------------------
// Ensure cv correctness
// ------------------------------

template <typename T>
struct numeric_limits<const T> : numeric_limits<T> {
};

template <typename T>
struct numeric_limits<volatile T> : numeric_limits<T> {
};

template <typename T>
struct numeric_limits<const volatile T> : numeric_limits<T> {
};

// ------------------------------
// Specializations
// ------------------------------

template <class T>
    requires(is_integral_v<T> && is_unsigned_v<T> && !is_same_v<T, bool>)
struct numeric_limits<T> {
    // ------------------------------
    // Constant Fields
    // ------------------------------

    static constexpr bool is_specialized = true;

    static constexpr bool is_signed  = false;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact   = true;

    static constexpr int digits       = sizeof(byte) * sizeof(T);
    static constexpr int digits10     = ::internal::digits10(digits);
    static constexpr int max_digits10 = 0;

    static constexpr int radix       = 2;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo  = true;
    static constexpr bool traps      = true;

    // ------------------------------
    // Float constants
    // ------------------------------

    static constexpr int min_exponent              = 0;
    static constexpr int min_exponent10            = 0;
    static constexpr int max_exponent              = 0;
    static constexpr int max_exponent10            = 0;
    static constexpr bool has_infinity             = false;
    static constexpr bool has_quiet_NaN            = false;
    static constexpr bool has_signaling_NaN        = false;
    static constexpr bool has_denorm_loss          = false;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool is_iec559                = false;
    static constexpr float_round_style round_style = round_toward_zero;
    static constexpr bool tinyness_before          = false;

    // ------------------------------
    // Functions
    // ------------------------------

    static constexpr T min() noexcept { return static_cast<T>(0ull); }
    static constexpr T max() noexcept { return min() - 1; }
    static constexpr T lowest() noexcept { return min(); }

    // ------------------------------
    // Float functions
    // ------------------------------

    static constexpr T epsilon() noexcept { return 0; }
    static constexpr T round_error() noexcept { return 0; }
    static constexpr T infinity() noexcept { return 0; }
    static constexpr T quiet_NaN() noexcept { return 0; }
    static constexpr T signaling_NaN() noexcept { return 0; }
    static constexpr T denorm_min() noexcept { return 0; }
};

template <class T>
    requires(is_integral_v<T> && !is_unsigned_v<T> && !is_same_v<T, bool>)
struct numeric_limits<T> {
    // ------------------------------
    // Constant Fields
    // ------------------------------

    static constexpr bool is_specialized = true;

    static constexpr bool is_signed  = true;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact   = true;

    static constexpr int digits       = sizeof(byte) * sizeof(T) - 1;  // Sign bit is not counted
    static constexpr int digits10     = ::internal::digits10(digits);
    static constexpr int max_digits10 = 0;

    static constexpr int radix       = 2;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo  = false;
    static constexpr bool traps      = true;

    // ------------------------------
    // Float constants
    // ------------------------------

    static constexpr int min_exponent              = 0;
    static constexpr int min_exponent10            = 0;
    static constexpr int max_exponent              = 0;
    static constexpr int max_exponent10            = 0;
    static constexpr bool has_infinity             = false;
    static constexpr bool has_quiet_NaN            = false;
    static constexpr bool has_signaling_NaN        = false;
    static constexpr bool has_denorm_loss          = false;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool is_iec559                = false;
    static constexpr float_round_style round_style = round_toward_zero;
    static constexpr bool tinyness_before          = false;

    // ------------------------------
    // Functions
    // ------------------------------

    static constexpr T min() noexcept { return kMsb<T>; }
    static constexpr T max() noexcept
    {
        return kFullMask<T> ^ kMsb<T>;  // All bits set except sign bit
    }
    static constexpr T lowest() noexcept { return min(); }

    // ------------------------------
    // Float functions
    // ------------------------------

    static constexpr T epsilon() noexcept { return 0; }
    static constexpr T round_error() noexcept { return 0; }
    static constexpr T infinity() noexcept { return 0; }
    static constexpr T quiet_NaN() noexcept { return 0; }
    static constexpr T signaling_NaN() noexcept { return 0; }
    static constexpr T denorm_min() noexcept { return 0; }
};

template <>
struct numeric_limits<bool> {
    // ------------------------------
    // Constant Fields
    // ------------------------------

    static constexpr bool is_specialized = true;

    static constexpr bool is_signed  = false;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact   = true;

    static constexpr int digits       = 1;
    static constexpr int digits10     = 0;
    static constexpr int max_digits10 = 0;

    static constexpr int radix       = 2;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo  = false;
    static constexpr bool traps      = true;

    // ------------------------------
    // Float constants
    // ------------------------------

    static constexpr int min_exponent              = 0;
    static constexpr int min_exponent10            = 0;
    static constexpr int max_exponent              = 0;
    static constexpr int max_exponent10            = 0;
    static constexpr bool has_infinity             = false;
    static constexpr bool has_quiet_NaN            = false;
    static constexpr bool has_signaling_NaN        = false;
    static constexpr bool has_denorm_loss          = false;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool is_iec559                = false;
    static constexpr float_round_style round_style = round_toward_zero;
    static constexpr bool tinyness_before          = false;

    // ------------------------------
    // Functions
    // ------------------------------

    static constexpr bool min() noexcept { return false; }
    static constexpr bool max() noexcept { return true; }
    static constexpr bool lowest() noexcept { return min(); }

    // ------------------------------
    // Float functions
    // ------------------------------

    static constexpr bool epsilon() noexcept { return false; }
    static constexpr bool round_error() noexcept { return false; }
    static constexpr bool infinity() noexcept { return false; }
    static constexpr bool quiet_NaN() noexcept { return false; }
    static constexpr bool signaling_NaN() noexcept { return false; }
    static constexpr bool denorm_min() noexcept { return false; }
};

TODO_LIBCPP_COMPLIANCE
// TODO: Add floats...
// TODO: Recheck various chars

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_LIMITS_HPP_
