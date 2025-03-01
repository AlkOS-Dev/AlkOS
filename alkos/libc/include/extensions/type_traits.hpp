#ifndef LIBC_INCLUDE_EXTENSIONS_TYPE_TRAITS_HPP_
#define LIBC_INCLUDE_EXTENSIONS_TYPE_TRAITS_HPP_

#include <todo.h>
#include <extensions/cstddef.hpp>
#include <extensions/defines.hpp>

#define __DEF_COMPILER_DEF_TYPE_TRAIT(name, func) \
    template <typename T>                         \
    struct name : bool_constant<func(T)> {        \
    };                                            \
                                                  \
    template <class T>                            \
    constexpr bool name##_v = name<T>::value;

#define __DEF_CONSTEXPR_ACCESSOR(name) \
    template <class T>                 \
    constexpr bool name##_v = name<T>::value;

#define __DEF_CONSTEXPR_ACCESSOR_T(name) \
    template <class T>                   \
    using name##_t = typename name<T>::type;

TODO_LIBCPP_COMPLIANCE
/**
 * TODO: Missing implementations:
 * - std::common_reference
 * - std::common_type
 * - std::is_swappable
 * - std::is_swappable_with
 * - std::is_nothrow_swappable
 * - std::is_nothrow_swappable_with
 * - std::is_destructible
 * - std::is_nothrow_destructible
 * - std::is_trivially_destructible
 */

namespace std
{
// ------------------------------
// Forward Declarations
// ------------------------------

template <class T>
struct reference_wrapper;

// ------------------------------
// Internal helpers
// ------------------------------

namespace internal
{
template <class T>
constexpr const T &max(const T &a, const T &b)
{
    return a < b ? b : a;
}

template <class T, class U = T &&>
U declval_base(int) noexcept;

template <class T>
T declval_base(...) noexcept;

template <typename T>
auto declval() noexcept -> decltype(internal::declval_base<T>(0))
{
    return internal::declval_base<T>();
}

template <class>
constexpr bool is_reference_wrapper_v = false;

template <class T>
constexpr bool is_reference_wrapper_v<reference_wrapper<T>> = true;

struct failure_type {
    /* No type field should cause a compile-time error */
};

template <class T>
struct type_wrap {
    using type = T;
};
}  // namespace internal

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper Classes
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::integral_constant
// ------------------------------

template <class T, T v>
struct integral_constant {
    static constexpr T value = v;
    using value_type         = T;
    using type               = integral_constant;
    NODISCARD FORCE_INLINE_F constexpr operator value_type() const noexcept { return value; }
    NODISCARD FORCE_INLINE_F constexpr value_type operator()() const noexcept { return value; }
};

// ------------------------------
// bool integral constants
// ------------------------------

template <bool B>
using bool_constant = integral_constant<bool, B>;

using true_type  = bool_constant<true>;
using false_type = bool_constant<false>;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Miscellaneous transformations
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::is_same
// ------------------------------

template <class X, class Y>
struct is_same : false_type {
};

template <class X>
struct is_same<X, X> : true_type {
};

template <class X, class Y>
constexpr bool is_same_v = is_same<X, Y>::value;

// ------------------------------
// std::type_identity
// ------------------------------

template <class T>
struct type_identity {
    using type = T;
};

__DEF_CONSTEXPR_ACCESSOR_T(type_identity)

// ------------------------------
// std::conditional
// ------------------------------

template <bool B, class X, class Y>
struct conditional {
    using type = X;
};

template <class X, class Y>
struct conditional<false, X, Y> {
    using type = Y;
};

template <bool B, class X, class Y>
using conditional_t = typename conditional<B, X, Y>::type;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Const-volatility specifiers
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::remove_cv
// ------------------------------

template <class T>
struct remove_cv : type_identity<T> {
};

template <class T>
struct remove_cv<const T> : type_identity<T> {
};

template <class T>
struct remove_cv<volatile T> : type_identity<T> {
};

template <class T>
struct remove_cv<const volatile T> : type_identity<T> {
};

__DEF_CONSTEXPR_ACCESSOR_T(remove_cv)

// ------------------------------
// std::remove_const
// ------------------------------

template <class T>
struct remove_const : type_identity<T> {
};

template <class T>
struct remove_const<const T> {
    using type = T;
};

__DEF_CONSTEXPR_ACCESSOR_T(remove_const)

// ------------------------------
// std::remove_volatile
// ------------------------------

template <class T>
struct remove_volatile : type_identity<T> {
};

template <class T>
struct remove_volatile<volatile T> {
    using type = T;
};

__DEF_CONSTEXPR_ACCESSOR_T(remove_volatile)

// ------------------------------
// std::add_cv
// ------------------------------

template <class T>
struct add_cv {
    using type = const volatile T;
};

__DEF_CONSTEXPR_ACCESSOR_T(add_cv)

// ------------------------------
// std::add_const
// ------------------------------

template <class T>
struct add_const {
    using type = const T;
};

__DEF_CONSTEXPR_ACCESSOR_T(add_const)

// ------------------------------
// std::add_volatile
// ------------------------------

template <class T>
struct add_volatile {
    using type = volatile T;
};

__DEF_CONSTEXPR_ACCESSOR_T(add_volatile)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// References
///////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace internal
{
template <class T>
auto try_add_lvalue_reference(int) -> type_identity<T &>
{
    return {};
}

template <class T>
auto try_add_lvalue_reference(...) -> type_identity<T>
{
    return {};
}

template <class T>
auto try_add_rvalue_reference(int) -> type_identity<T &&>
{
    return {};
}

template <class T>
auto try_add_rvalue_reference(...) -> type_identity<T>
{
    return {};
}
}  // namespace internal

// ------------------------------
// std::remove_reference
// ------------------------------

template <class T>
struct remove_reference {
    using type = T;
};

template <class T>
struct remove_reference<T &> {
    using type = T;
};

template <class T>
struct remove_reference<T &&> {
    using type = T;
};

__DEF_CONSTEXPR_ACCESSOR_T(remove_reference)

// -------------------------------
// std::add_lvalue_reference
// -------------------------------

template <class T>
struct add_lvalue_reference : decltype(internal::try_add_lvalue_reference<T>(0)) {
};

__DEF_CONSTEXPR_ACCESSOR_T(add_lvalue_reference)

// -------------------------------
// std::add_rvalue_reference
// -------------------------------

template <class T>
struct add_rvalue_reference : decltype(internal::try_add_rvalue_reference<T>(0)) {
};

__DEF_CONSTEXPR_ACCESSOR_T(add_rvalue_reference)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Pointers
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::remove_pointer
// ------------------------------

template <class T>
struct remove_pointer : type_identity<T> {
};

template <class T>
struct remove_pointer<T *> : type_identity<T> {
};

template <class T>
struct remove_pointer<T *const> : type_identity<T> {
};

template <class T>
struct remove_pointer<T *volatile> : type_identity<T> {
};

template <class T>
struct remove_pointer<T *volatile const> : type_identity<T> {
};

__DEF_CONSTEXPR_ACCESSOR_T(remove_pointer)

// ------------------------------
// std::add_pointer
// ------------------------------

template <typename T>
struct add_pointer {
    using type = remove_reference_t<T> *;
};

__DEF_CONSTEXPR_ACCESSOR_T(add_pointer)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Primary type categories
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::is_void
// ------------------------------

template <class T>
struct is_void : std::is_same<void, std::remove_cv_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_void)

// ------------------------------
// std::is_null_pointer
// ------------------------------

template <class T>
struct is_null_pointer : std::is_same<std::nullptr_t, std::remove_cv_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_null_pointer)

// ------------------------------
// std::is_integral
// ------------------------------

namespace internal
{
template <class T>
struct is_integral
    : std::bool_constant<
          std::is_same_v<T, int> || std::is_same_v<T, bool> || std::is_same_v<T, char> ||
          std::is_same_v<T, signed char> || std::is_same_v<T, short> || std::is_same_v<T, long> ||
          std::is_same_v<T, long long> || std::is_same_v<T, char8_t> ||
          std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t> ||
          std::is_same_v<T, wchar_t> || std::is_same_v<T, unsigned int> ||
          std::is_same_v<T, unsigned char> || std::is_same_v<T, unsigned short> ||
          std::is_same_v<T, unsigned long> || std::is_same_v<T, unsigned long long>> {
};
}  // namespace internal

template <class T>
struct is_integral : internal::is_integral<std::remove_cv_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_integral)

// ------------------------------
// std::is_floating_point
// ------------------------------

namespace internal
{
template <class T>
struct is_floating_point
    : std::bool_constant<
          std::is_same_v<T, float> || std::is_same_v<T, double> || std::is_same_v<T, long double>> {
};
}  // namespace internal

template <class T>
struct is_floating_point : internal::is_floating_point<std::remove_cv_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_floating_point)

// ------------------------------
// std::is_array
// ------------------------------

template <class T>
struct is_array : false_type {
};

template <class T>
struct is_array<T[]> : true_type {
};

template <class T, size_t N>
struct is_array<T[N]> : true_type {
};

__DEF_CONSTEXPR_ACCESSOR(is_array)

// ------------------------------
// std::is_enum
// ------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_enum, __is_enum)

// ------------------------------
// std::is_union
// ------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_union, __is_union)

// ------------------------------
// std::is_class
// ------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_class, __is_class)

// ------------------------------
// std::is_pointer
// ------------------------------

namespace internal
{
template <class T>
struct is_pointer : std::false_type {
};

template <class T>
struct is_pointer<T *> : std::true_type {
};
}  // namespace internal

template <class T>
struct is_pointer : internal::is_pointer<std::remove_cv_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_pointer)

// ------------------------------
// std::is_lvalue_reference
// ------------------------------

template <typename>
struct is_lvalue_reference : false_type {
};

template <typename _Tp>
struct is_lvalue_reference<_Tp &> : true_type {
};

__DEF_CONSTEXPR_ACCESSOR(is_lvalue_reference)

// ------------------------------
// std::is_rvalue_reference
// ------------------------------

template <typename>
struct is_rvalue_reference : false_type {
};

template <typename _Tp>
struct is_rvalue_reference<_Tp &&> : true_type {
};

__DEF_CONSTEXPR_ACCESSOR(is_rvalue_reference)

// ------------------------------
// std::is_function
// ------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_function, __is_function)

// -------------------------------------
// std::is_member_function_pointer
// -------------------------------------

namespace internal
{
template <class T>
struct is_member_function_pointer : std::false_type {
};

template <class T, class U>
struct is_member_function_pointer<T U::*> : std::is_function<T> {
};
}  // namespace internal

template <class T>
struct is_member_function_pointer : internal::is_member_function_pointer<remove_cv_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_member_function_pointer)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Composite Types
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::is_arithmetic
// ------------------------------

template <class T>
struct is_arithmetic
    : std::integral_constant<bool, std::is_integral_v<T> || std::is_floating_point_v<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_arithmetic)

// ------------------------------
// std::is_fundamental
// ------------------------------

template <class T>
struct is_fundamental
    : std::bool_constant<
          std::is_arithmetic_v<T> || std::is_void_v<T> || std::is_null_pointer_v<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_fundamental)

// ------------------------------
// std::is_member_pointer
// ------------------------------

namespace internal
{
template <class T>
struct is_member_pointer : std::false_type {
};

template <class T, class U>
struct is_member_pointer<T U::*> : std::true_type {
};
}  // namespace internal

template <class T>
struct is_member_pointer : internal::is_member_pointer<std::remove_cv_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_member_pointer)

// ------------------------------
// std::is_scalar
// ------------------------------

template <class T>
struct is_scalar : std::integral_constant<
                       bool, std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_pointer_v<T> ||
                                 std::is_member_pointer_v<T> || std::is_null_pointer_v<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_scalar)

// ------------------------------
// std::is_object
// ------------------- -----------

template <class T>
struct is_object
    : std::bool_constant<
          std::is_scalar_v<T> || std::is_array_v<T> || std::is_union_v<T> || std::is_class_v<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_object)

// ------------------------------
// std::is_compound
// ------------------------------

template <class T>
struct is_compound : std::bool_constant<!std::is_fundamental_v<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_compound)

// ------------------------------
// std::is_reference
// ------------------------------

template <class T>
struct is_reference : false_type {
};

template <class T>
struct is_reference<T &> : true_type {
};

template <class T>
struct is_reference<T &&> : true_type {
};

__DEF_CONSTEXPR_ACCESSOR(is_reference)

// -----------------------------------
// std::is_member_object_pointer
// -----------------------------------

template <class T>
struct is_member_object_pointer
    : std::bool_constant<std::is_member_pointer_v<T> && !std::is_member_function_pointer_v<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_member_object_pointer)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Type properties
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::is_const
// ------------------------------

template <class T>
struct is_const : false_type {
};

template <class T>
struct is_const<const T> : true_type {
};

__DEF_CONSTEXPR_ACCESSOR(is_const)

// ------------------------------
// std::is_volatile
// ------------------------------

template <class T>
struct is_volatile : std::false_type {
};

template <class T>
struct is_volatile<volatile T> : std::true_type {
};

__DEF_CONSTEXPR_ACCESSOR(is_volatile)

// ------------------------------
// std::is_unbounded_array
// ------------------------------

template <class T>
struct is_unbounded_array : std::false_type {
};

template <class T>
struct is_unbounded_array<T[]> : std::true_type {
};

__DEF_CONSTEXPR_ACCESSOR(is_unbounded_array)

// ------------------------------
// std::is_bounded_array
// ------------------------------

template <class T>
struct is_bounded_array : std::false_type {
};

template <class T, std::size_t N>
struct is_bounded_array<T[N]> : std::true_type {
};

__DEF_CONSTEXPR_ACCESSOR(is_bounded_array)

// ------------------------------
// std::is_unsigned
// ------------------------------

template <class T>
struct is_unsigned : std::bool_constant<std::is_arithmetic_v<T> && T(0) < T(-1)> {
};

__DEF_CONSTEXPR_ACCESSOR(is_unsigned)

// ------------------------------
// std::is_unsigned
// ------------------------------

template <class T>
struct is_signed : std::bool_constant<std::is_arithmetic_v<T> && T(-1) < T(0)> {
};

__DEF_CONSTEXPR_ACCESSOR(is_signed)

// ------------------------------
// std::is_final
// ------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_final, __is_final)

// ------------------------------
// std::is_abstract
// ------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_abstract, __is_abstract)

// ------------------------------
// std::is_polymorphic
// ------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_polymorphic, __is_polymorphic)

// ------------------------------
// std::is_empty
// ------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_empty, __is_empty)

// ------------------------------
// std::is_aggregate
// ------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_aggregate, __is_aggregate)

// --------------------------------------------
// std::has_unique_object_representations
// --------------------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(
    has_unique_object_representations, __has_unique_object_representations
)

// ------------------------------
// std::is_standard_layout
// ------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_standard_layout, __is_standard_layout)

// --------------------------------
// std::is_trivially_copyable
// --------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_trivially_copyable, __is_trivially_copyable)

// ------------------------------
// std::is_trivial
// ------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_trivial, __is_trivial)

// ------------------------------
// std::is_pod
// ------------------------------

__DEF_COMPILER_DEF_TYPE_TRAIT(is_pod, __is_pod)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sign modifiers
///////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace internal
{
template <class T, bool IsConst, bool IsVolatile>
struct preserve_cv_base {
    using type = T;
};

template <class T>
struct preserve_cv_base<T, true, false> {
    using type = const T;
};

template <class T>
struct preserve_cv_base<T, false, true> {
    using type = volatile T;
};

template <class T>
struct preserve_cv_base<T, true, true> {
    using type = const volatile T;
};

template <class QualifiedT, class UnqualifiedT>
struct preserve_cv {
    using type = typename preserve_cv_base<
        UnqualifiedT, std::is_const_v<QualifiedT>, std::is_volatile_v<QualifiedT>>::type;
};

template <class T>
struct lowest_rank_unsigned_selector {
    private:
    static_assert(
        sizeof(T) <= sizeof(unsigned long long),
        "Given type is bigger than all possible integer types"
    );

    template <size_t kSize, class U, class... Types>
    struct select_ {
        using type =
            std::conditional_t<kSize <= sizeof(U), U, typename select_<kSize, Types...>::type>;
    };

    public:
    using type = typename select_<
        sizeof(T), unsigned char, unsigned short, unsigned int, unsigned long,
        unsigned long long>::type;
};
}  // namespace internal

// ------------------------------
// std::make_unsigned
// ------------------------------

namespace internal
{
template <class T>
struct make_unsigned_from_cv_cleaned {
    using type = T;
};

template <>
struct make_unsigned_from_cv_cleaned<signed char> {
    using type = unsigned char;
};

template <>
struct make_unsigned_from_cv_cleaned<short> {
    using type = unsigned short;
};

template <>
struct make_unsigned_from_cv_cleaned<int> {
    using type = unsigned int;
};

template <>
struct make_unsigned_from_cv_cleaned<long> {
    using type = unsigned long;
};

template <>
struct make_unsigned_from_cv_cleaned<long long> {
    using type = unsigned long long;
};

template <>
struct make_unsigned_from_cv_cleaned<char> {
    using type = unsigned char;
};

template <class T, bool kIsIntegral = std::is_integral_v<T>, bool kIsEnum = std::is_enum_v<T>>
struct make_unsigned;

template <class T>
struct make_unsigned<T, true, false> {
    private:
    using unsigned_t = typename make_unsigned_from_cv_cleaned<std::remove_cv_t<T>>::type;

    public:
    using type = typename preserve_cv<T, unsigned_t>::type;
};

template <class T>
struct make_unsigned<T, false, true> {
    protected:
    using unsigned_type = typename lowest_rank_unsigned_selector<remove_cv_t<T>>::type;

    public:
    using type = typename preserve_cv<T, unsigned_type>::type;
};
}  // namespace internal

template <class T>
struct make_unsigned : internal::make_unsigned<T> {
};

__DEF_CONSTEXPR_ACCESSOR_T(make_unsigned)

// ------------------------------
// std::make_signed
// ------------------------------

namespace internal
{
template <class T>
struct make_signed_from_cv_cleaned {
    using type = T;
};

template <>
struct make_signed_from_cv_cleaned<unsigned char> {
    using type = signed char;
};

template <>
struct make_signed_from_cv_cleaned<unsigned short> {
    using type = short;
};

template <>
struct make_signed_from_cv_cleaned<unsigned int> {
    using type = int;
};

template <>
struct make_signed_from_cv_cleaned<unsigned long> {
    using type = long;
};

template <>
struct make_signed_from_cv_cleaned<unsigned long long> {
    using type = long long;
};

template <>
struct make_signed_from_cv_cleaned<char> {
    using type = signed char;
};

template <class T, bool kIsIntegral = std::is_integral_v<T>, bool kIsEnum = std::is_enum_v<T>>
struct make_signed;

template <class T>
struct make_signed<T, true, false> {
    private:
    using signed_t = typename make_signed_from_cv_cleaned<std::remove_cv_t<T>>::type;

    public:
    using type = typename preserve_cv<T, signed_t>::type;
};

template <class T>
struct make_signed<T, false, true> {
    protected:
    using unsigned_type = typename lowest_rank_unsigned_selector<remove_cv_t<T>>::type;
    using signed_type   = typename make_signed<unsigned_type>::type;

    public:
    using type = typename preserve_cv<T, signed_type>::type;
};
}  // namespace internal

template <class T>
struct make_signed : internal::make_signed<T> {
};

__DEF_CONSTEXPR_ACCESSOR_T(make_signed)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Arrays
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::remove_extent
// ------------------------------

template <class T>
struct remove_extent : type_identity<T> {
};

template <class T>
struct remove_extent<T[]> : type_identity<T> {
};

template <class T, size_t N>
struct remove_extent<T[N]> : type_identity<T> {
};

__DEF_CONSTEXPR_ACCESSOR_T(remove_extent)

// ------------------------------
// std::remove_all_extents
// ------------------------------

template <class T>
struct remove_all_extents {
    using type = T;
};

template <class T>
struct remove_all_extents<T[]> {
    using type = typename remove_all_extents<T>::type;
};

template <class T, std::size_t N>
struct remove_all_extents<T[N]> {
    using type = typename remove_all_extents<T>::type;
};

__DEF_CONSTEXPR_ACCESSOR_T(remove_all_extents)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Miscellaneous transformations 2
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::decay
// ------------------------------

// clang-format off
    template <class T>
    struct decay {
    private:
        using U = remove_reference_t<T>;

    public:
        using type = conditional_t<
            is_array_v<U>,
            add_pointer_t<remove_extent_t<U>>,
            conditional_t<
                is_function_v<U>,
                add_pointer_t<U>,
                remove_cv_t<U>
            >
        >;
    };
// clang-format on

template <class T>
using decay_t = typename decay<T>::type;

// ------------------------------
// std::aligned_storage
// ------------------------------

template <std::size_t kLen, std::size_t kAlign = alignof(std::size_t)>
struct aligned_storage {
    struct type {
        alignas(kAlign) unsigned char data[kLen];
    };
};

template <std::size_t kLen, std::size_t kAlign = alignof(std::size_t)>
using aligned_storage_t = typename aligned_storage<kLen, kAlign>::type;

// ------------------------------
// std::aligned_union
// ------------------------------

template <std::size_t kLen, class T, class... Types>
struct aligned_union {
    static constexpr std::size_t alignment_value =
        internal::max(alignof(T), aligned_union<kLen, Types...>::alignment_value);

    static constexpr std::size_t clamped_size =
        internal::max(sizeof(T), aligned_union<kLen, Types...>::clamped_size);

    struct type {
        alignas(alignment_value) char _s[internal::max(kLen, clamped_size)];
    };
};

template <std::size_t kLen, class T>
struct aligned_union<kLen, T> {
    static constexpr std::size_t alignment_value = alignof(T);
    static constexpr std::size_t clamped_size    = sizeof(T);

    struct type {
        alignas(alignment_value) char _s[internal::max(kLen, clamped_size)];
    };
};

template <std::size_t kSize, class... Types>
using aligned_union_t = typename aligned_union<kSize, Types...>::type;

// ------------------------------
// std::enable_if
// ------------------------------

template <bool, class>
struct enable_if {
};

template <class T>
struct enable_if<true, T> {
    using type = T;
};

template <bool kEnable, class T>
using enable_if_t = typename enable_if<kEnable, T>::type;

// ------------------------------
// std::remove_cvref
// ------------------------------

template <class T>
struct remove_cvref {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

__DEF_CONSTEXPR_ACCESSOR_T(remove_cvref)

// ------------------------------
// std::void_t
// ------------------------------

template <typename...>
using void_t = void;

// ------------------------------
// std::unwrap_reference
// ------------------------------

template <class T>
struct unwrap_reference {
    using type = T;
};

template <class T>
struct unwrap_reference<std::reference_wrapper<T>> {
    using type = T &;
};

__DEF_CONSTEXPR_ACCESSOR_T(unwrap_reference)

// ------------------------------
// std::unwrap_ref_decay
// ------------------------------

template <class T>
struct unwrap_ref_decay : std::unwrap_reference<std::decay_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR_T(unwrap_ref_decay)

// ------------------------------
// std::underlying_type
// ------------------------------

namespace internal
{
template <class T, bool = is_enum_v<T>>
struct underlying_type {
};

template <class T>
struct underlying_type<T, true> {
    using type = __underlying_type(T);
};
}  // namespace internal

template <class T>
struct underlying_type : internal::underlying_type<T> {
};

__DEF_CONSTEXPR_ACCESSOR_T(underlying_type)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Property queries
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::alignment_of
// ------------------------------

template <class T>
struct alignment_of : std::integral_constant<std::size_t, alignof(T)> {
};

template <class T>
constexpr std::size_t alignment_of_v = alignment_of<T>::value;

// ------------------------------
// std::rank
// ------------------------------

template <class T>
struct rank : std::integral_constant<std::size_t, 0> {
};

template <class T>
struct rank<T[]> : std::integral_constant<std::size_t, rank<T>::value + 1> {
};

template <class T, std::size_t N>
struct rank<T[N]> : std::integral_constant<std::size_t, rank<T>::value + 1> {
};

template <class T>
constexpr std::size_t rank_v = rank<T>::value;

// ------------------------------
// std::extent
// ------------------------------

template <class T, unsigned = 0>
struct extent : std::integral_constant<std::size_t, 0> {
};

template <class T>
struct extent<T[], 0> : std::integral_constant<std::size_t, 0> {
};

template <class T, unsigned N>
struct extent<T[], N> : std::extent<T, N - 1> {
};

template <class T, std::size_t I>
struct extent<T[I], 0> : std::integral_constant<std::size_t, I> {
};

template <class T, std::size_t I, unsigned N>
struct extent<T[I], N> : std::extent<T, N - 1> {
};

template <class T, unsigned N = 0>
constexpr std::size_t extent_v = extent<T, N>::value;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------
// std::is_constant_evaluated()
// ----------------------------------

NODISCARD constexpr bool is_constant_evaluated() noexcept
{
    return __builtin_is_constant_evaluated();
}

// ----------------------------------
// std::is_corresponding_member
// ----------------------------------

template <typename S1, typename S2, typename M1, typename M2>
NODISCARD constexpr bool is_corresponding_member(M1 S1::*m1, M2 S2::*m2) noexcept
{
    return __builtin_is_corresponding_member(m1, m2);
}

// --------------------------------------------
// is_pointer_interconvertible_with_class
// --------------------------------------------

template <typename T, typename Mem>
NODISCARD constexpr bool is_pointer_interconvertible_with_class(Mem T::*mp) noexcept
{
    return __builtin_is_pointer_interconvertible_with_class(mp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Operations on traits
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::conjunction
// ------------------------------

template <class...>
struct conjunction : std::true_type {
};

template <class T>
struct conjunction<T> : T {
};

template <class T, class... Types>
struct conjunction<T, Types...> : std::conditional_t<T::value, conjunction<Types...>, T> {
};

template <class... Types>
constexpr bool conjunction_v = conjunction<Types...>::value;

// ------------------------------
// std::disjunction
// ------------------------------

template <class...>
struct disjunction : std::false_type {
};

template <class T, class... Types>
struct disjunction<T, Types...> : std::conditional_t<T::value, T, disjunction<Types...>> {
};

template <class T>
struct disjunction<T> : T {
};

template <class... Types>
constexpr bool disjunction_v = disjunction<Types...>::value;

// ------------------------------
// std::negation
// ------------------------------

template <class B>
struct negation : std::bool_constant<!B::value> {
};

__DEF_CONSTEXPR_ACCESSOR(negation)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Type relationships
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::is_base_of
// ------------------------------

template <class Base, class Derived>
struct is_base_of : std::bool_constant<__is_base_of(Base, Derived)> {
};

template <class Base, class Derived>
constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;

// ------------------------------
// std::is_convertible
// ------------------------------

namespace internal
{
template <class T>
auto is_returnable(int) -> decltype(void(static_cast<T (*)()>(nullptr)), std::true_type{})
{
    return {};
}

template <class>
auto is_returnable(...) -> std::false_type
{
    return {};
}

template <class From, class To>
auto is_implicitly_convertible(int
) -> decltype(void(internal::declval<void (&)(To)>()(internal::declval<From>())), std::true_type{})
{
    return {};
}

template <class, class>
auto is_implicitly_convertible(...) -> std::false_type
{
    return {};
}
};  // namespace internal

template <class From, class To>
struct is_convertible : std::bool_constant<
                            (decltype(internal::is_returnable<To>(0))::value &&
                             decltype(internal::is_implicitly_convertible<From, To>(0))::value) ||
                            (std::is_void_v<From> && std::is_void_v<To>)> {
};

template <class From, class To>
constexpr bool is_convertible_v = is_convertible<From, To>::value;

// ---------------------------------
// std::is_nothrow_convertible
// ---------------------------------

template <typename From, typename To>
constexpr bool is_nothrow_convertible_v = __is_nothrow_convertible(From, To);

template <typename From, typename To>
struct is_nothrow_convertible : bool_constant<is_nothrow_convertible_v<From, To>> {
};

// -------------------------------
// std::is_layout_compatible
// -------------------------------

template <class T, class U>
constexpr bool is_layout_compatible_v = __is_layout_compatible(T, U);

template <class T, class U>
struct is_layout_compatible : bool_constant<is_layout_compatible_v<T, U>> {
};

// ----------------------------------------------
// std::is_pointer_interconvertible_base_of
// ----------------------------------------------

template <class Base, class Derived>
constexpr bool is_pointer_interconvertible_base_of_v =
    __is_pointer_interconvertible_base_of(Base, Derived);

template <class Base, class Derived>
struct is_pointer_interconvertible_base_of
    : bool_constant<is_pointer_interconvertible_base_of_v<Base, Derived>> {
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Transformation cont...
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------
// std::invoke_result
// ------------------------------

namespace internal
{
template <bool kIsFunctor, bool kIsMemberFunction, class Fn, class... Args>
struct inspect_invoke_typed {
    /* Invalid type for true, true specialization - no type field */
};

template <class MemPtr, class Arg>
struct invoke_mem_obj;

template <class MemPtr, class ArgT>
struct invoke_mem_obj_ref {
    private:
    template <class F, class T>
    static type_wrap<decltype(declval<T>().*declval<F>())> test(int)
    {
        return {};
    }

    template <class, class>
    static failure_type test(...)
    {
        return {};
    }

    template <class F, class T>
    constexpr static bool test_noexcept(int) noexcept
    {
        return noexcept(declval<T>().*declval<F>());
    }

    template <class, class>
    constexpr static bool test_noexcept(...)
    {
        return false;
    }

    public:
    using type                    = decltype(test<MemPtr, ArgT>(0));
    static constexpr bool nothrow = test_noexcept<MemPtr, ArgT>(0);
};

template <class MemPtr, class ArgT>
struct invoke_mem_obj_deref {
    private:
    template <class F, class T>
    static type_wrap<decltype(((*declval<T>()).*declval<F>())())> test(int)
    {
        return {};
    }

    template <class, class>
    static failure_type test(...)
    {
        return {};
    }

    template <class F, class T>
    static bool test_noexcept(int) noexcept
    {
        return noexcept(((*declval<T>()).*declval<F>())());
    }

    template <class, class>
    static bool test_noexcept(...)
    {
        return false;
    }

    public:
    using type                    = decltype(test<MemPtr, ArgT>(0));
    static constexpr bool nothrow = test_noexcept<MemPtr, ArgT>(0);
};

template <class Result, class Class, class Arg>
struct invoke_mem_obj<Result Class::*, Arg> {
    using arg_t     = std::remove_cvref_t<Arg>;
    using mem_ptr_t = Result Class::*;
    using type      = typename std::conditional_t<
             std::is_same_v<arg_t, Class> || std::is_base_of_v<Class, arg_t>,
             invoke_mem_obj_ref<mem_ptr_t, arg_t>, invoke_mem_obj_deref<mem_ptr_t, arg_t>>::type;
};

template <class Fn, class Arg>
struct inspect_invoke_typed<true, false, Fn, Arg>
    : invoke_mem_obj<std::decay_t<Fn>, std::decay_t<Arg>> {
    /* Member object pointer */
};

template <class MemPtr, class Arg, class... Args>
struct invoke_mem_fun;

template <class MemPtr, class Arg, class... Args>
struct invoke_mem_fun_ref {
    private:
    template <class F, class T, class... ArgsT>
    static type_wrap<decltype(declval<T>().*declval<F>()(declval<ArgsT>()...))> test(int)
    {
        return {};
    }

    template <class, class, class...>
    static failure_type test(...)
    {
        return {};
    }

    template <class F, class T, class... ArgsT>
    constexpr static bool test_noexcept(int) noexcept
    {
        return noexcept(declval<T>().*declval<F>()(declval<ArgsT>()...));
    }

    template <class, class, class...>
    constexpr static bool test_noexcept(...)
    {
        return false;
    }

    public:
    using type                    = decltype(test<MemPtr, Arg, Args...>(0));
    static constexpr bool nothrow = test_noexcept<MemPtr, Arg, Args...>(0);
};

template <class MemPtr, class Arg, class... Args>
struct invoke_mem_fun_deref {
    private:
    template <class F, class T, class... ArgsT>
    static type_wrap<decltype(((*declval<T>()).*declval<F>())(declval<ArgsT>()...))> test(int)
    {
        return {};
    }

    template <class, class, class...>
    static failure_type test(...)
    {
        return {};
    }

    template <class F, class T, class... ArgsT>
    constexpr static bool test_noexcept(int) noexcept
    {
        return noexcept(((*declval<T>()).*declval<F>())(declval<ArgsT>()...));
    }

    template <class, class, class...>
    constexpr static bool test_noexcept(...)
    {
        return false;
    }

    public:
    using type                    = decltype(test<MemPtr, Arg, Args...>(0));
    static constexpr bool nothrow = test_noexcept<MemPtr, Arg, Args...>(0);
};

template <class Result, class Class, class Arg, class... Args>
struct invoke_mem_fun<Result Class::*, Arg, Args...> {
    using arg_t     = std::remove_reference_t<Arg>;
    using mem_ptr_t = Result Class::*;
    using type      = typename std::conditional_t<
             std::is_base_of_v<Class, arg_t>, invoke_mem_fun_ref<mem_ptr_t, arg_t, Args...>,
             invoke_mem_fun_deref<mem_ptr_t, arg_t, Args...>>::type;
};

template <class Fn, class Arg, class... Args>
struct inspect_invoke_typed<false, true, Fn, Arg, Args...>
    : invoke_mem_fun<std::decay_t<Fn>, remove_reference_t<Arg>, Args...> {
    /* Member function pointer */
};

template <class Fn, class... Args>
struct inspect_invoke_typed<false, false, Fn, Args...> {
    private:
    template <class F, class... ArgsT>
    static failure_type test(...)
    {
        return {};
    }

    template <class F, class... ArgsT>
    static auto test(int) -> type_wrap<decltype(declval<F>()(declval<ArgsT>()...))>
    {
        return {};
    }

    template <class F, class... ArgsT>
    constexpr static bool test_noexcept(int) noexcept
    {
        return noexcept(declval<F>()(declval<ArgsT>()...));
    }

    template <class F, class... ArgsT>
    constexpr static bool test_noexcept(...)
    {
        return false;
    }

    public:
    /* Usual function call */
    using type                    = decltype(test<Fn, Args...>(0));
    static constexpr bool nothrow = test_noexcept<Fn, Args...>(0);
};

template <typename Fn, typename... Args>
struct inspect_invoke
    : inspect_invoke_typed<
          std::is_member_object_pointer_v<std::remove_reference_t<Fn>>,
          std::is_member_function_pointer_v<std::remove_reference_t<Fn>>, Fn, Args...> {
    /* Expects type field for correctness */
};
}  // namespace internal

template <typename Fn, typename... Args>
struct invoke_result : internal::inspect_invoke<Fn, Args...>::type {
    static_assert(
        !std::is_same_v<
            typename internal::inspect_invoke<Fn, Args...>::type, internal::failure_type>,
        "Invalid invoke expression"
    );
};

template <class F, class... ArgTypes>
using invoke_result_t = typename invoke_result<F, ArgTypes...>::type;

// ------------------------------
// std::result_of
// ------------------------------

template <class>
struct result_of;

template <class F, class... Args>
struct result_of<F(Args...)> : internal::inspect_invoke<F, Args...>::type {
};

template <class T>
using result_of_t = typename result_of<T>::type;

// ------------------------------
// std::is_invocable
// ------------------------------

namespace internal
{
template <class Result, class Ret, bool = is_void_v<Ret>, class = void>
struct is_invocable : false_type {
    /* for _r */
    using nothrow = false_type;

    /* INVALID invoke expressions */
};

template <class Result, class Ret>
/* Correct only if Result contains type field */
struct is_invocable<Result, Ret, true, void_t<typename Result::type>> : true_type {
    /* for _r */
    using nothrow = true_type;

    /* VALID invoke expressions */
};

/* Implicit conversions to INVOKE<R> */
template <class Result, class Ret>
/* Correct only if Result contains type field */
struct is_invocable<Result, Ret, false, void_t<typename Result::type>> {
    private:
    // INVOKE<R> is valid

    using ResultType = typename Result::type;
    /* declval wout reference */
    static ResultType get() noexcept;

    /* test if some type is convertible to T */
    template <class T>
    static void test_impl_conv(type_identity_t<T>) noexcept
    {
    }

    template <
        class T, bool kNoexcept = noexcept(test_impl_conv<T>(get())),
        class        = decltype(test_impl_conv<T>(get())),
        bool kDangle = __reference_converts_from_temporary(T, ResultType)>
    static bool_constant<kNoexcept && !kDangle> test(int)
    {
        return {};
    }

    template <class, bool = false>
    static false_type test(...)
    {
        return {};
    }

    public:
    using type = decltype(test<Ret, true>(0));

    /* for _r */
    using nothrow = decltype(test<Ret>(0));
};
}  // namespace internal

template <class Fn, class... Args>
struct is_invocable
    : internal::is_invocable<typename internal::inspect_invoke<Fn, Args...>::type, void>::type {
};

template <class Fn, class... Args>
constexpr bool is_invocable_v = is_invocable<Fn, Args...>::value;

// ------------------------------
// std::is_invocable_r
// ------------------------------

template <class Ret, class Fn, class... Args>
struct is_invocable_r
    : internal::is_invocable<typename internal::inspect_invoke<Fn, Args...>::type, Ret>::type {
};

template <class Ret, class Fn, class... Args>
constexpr bool is_invocable_r_v = is_invocable_r<Ret, Fn, Args...>::value;

// ------------------------------
// std::is_nothrow_invocable
// ------------------------------

template <class Fn, class... Args>
struct is_nothrow_invocable
    : bool_constant<is_invocable_v<Fn, Args...> && internal::inspect_invoke<Fn, Args...>::nothrow> {
};

template <class Fn, class... Args>
constexpr bool is_nothrow_invocable_v = is_nothrow_invocable<Fn, Args...>::value;

// ------------------------------
// std::is_nothrow_invocable_r
// ------------------------------

template <class Ret, class Fn, class... Args>
struct is_nothrow_invocable_r
    : bool_constant<
          is_invocable_r_v<Ret, Fn, Args...> && internal::inspect_invoke<Fn, Args...>::nothrow> {
};

template <class Ret, class Fn, class... Args>
constexpr bool is_nothrow_invocable_r_v = is_nothrow_invocable_r<Ret, Fn, Args...>::value;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Supported operations
///////////////////////////////////////////////////////////////////////////////////////////////////////////

TODO_LIBCPP_COMPLIANCE
/**
 * TODO: Add static assert for complete type
 */

// ------------------------------
// Type construction
// ------------------------------

#define __DEF_COMPLETE_ARGS_TYPE_TRAIT(name, func)       \
    template <class T, class... Args>                    \
    struct name : std::bool_constant<func(T, Args...)> { \
    };                                                   \
                                                         \
    template <class T, class... Args>                    \
    constexpr bool name##_v = name<T, Args...>::value;

__DEF_COMPLETE_ARGS_TYPE_TRAIT(is_constructible, __is_constructible)
__DEF_COMPLETE_ARGS_TYPE_TRAIT(is_trivially_constructible, __is_trivially_constructible)
__DEF_COMPLETE_ARGS_TYPE_TRAIT(is_nothrow_constructible, __is_nothrow_constructible)

// -------------------------------
// Type default construction
// -------------------------------

template <class T>
struct is_default_constructible : std::is_constructible<T> {
};

__DEF_CONSTEXPR_ACCESSOR(is_default_constructible)

template <class T>
struct is_trivially_default_constructible : std::is_trivially_constructible<T> {
};

__DEF_CONSTEXPR_ACCESSOR(is_trivially_default_constructible)

template <class T>
struct is_nothrow_default_constructible : std::is_nothrow_constructible<T> {
};

__DEF_CONSTEXPR_ACCESSOR(is_nothrow_default_constructible)

// ------------------------------
// Type copy construction
// ------------------------------

template <class T>
struct is_copy_constructible : is_constructible<T, add_lvalue_reference_t<add_const_t<T>>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_copy_constructible)

template <class T>
struct is_trivially_copy_constructible
    : is_trivially_constructible<T, add_lvalue_reference_t<add_const_t<T>>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_trivially_copy_constructible)

template <class T>
struct is_nothrow_copy_constructible
    : is_nothrow_constructible<T, add_lvalue_reference_t<add_const_t<T>>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_nothrow_copy_constructible)

// ------------------------------
// Type move construction
// ------------------------------

template <class T>
struct is_move_constructible : is_constructible<T, add_rvalue_reference_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_move_constructible)

template <class T>
struct is_trivially_move_constructible : is_trivially_constructible<T, add_rvalue_reference_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_trivially_move_constructible)

template <class T>
struct is_nothrow_move_constructible : is_nothrow_constructible<T, add_rvalue_reference_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_nothrow_move_constructible)

// ------------------------------
// Type assignment
// ------------------------------

#define __DEF_COMPLETE_2_ARG_TYPE_TRAIT(name, func) \
    template <class T, class U>                     \
    struct name : std::bool_constant<func(T, U)> {  \
    };                                              \
                                                    \
    template <class T, class U>                     \
    constexpr bool name##_v = name<T, U>::value;

__DEF_COMPLETE_2_ARG_TYPE_TRAIT(is_assignable, __is_assignable)

__DEF_COMPLETE_2_ARG_TYPE_TRAIT(is_trivially_assignable, __is_trivially_assignable)

__DEF_COMPLETE_2_ARG_TYPE_TRAIT(is_nothrow_assignable, __is_nothrow_assignable)

// ------------------------------
// type copy assignment
// ------------------------------

template <class T>
struct is_copy_assignable
    : is_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<add_const_t<T>>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_copy_assignable)

template <class T>
struct is_trivially_copy_assignable
    : is_trivially_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<add_const_t<T>>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_trivially_copy_assignable)

template <class T>
struct is_nothrow_copy_assignable
    : is_nothrow_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<add_const_t<T>>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_nothrow_copy_assignable)

// ------------------------------
// Type move assignment
// ------------------------------

template <class T>
struct is_move_assignable : is_assignable<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_move_assignable)

template <class T>
struct is_trivially_move_assignable
    : is_trivially_assignable<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_trivially_move_assignable)

template <class T>
struct is_nothrow_move_assignable
    : is_nothrow_assignable<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>> {
};

__DEF_CONSTEXPR_ACCESSOR(is_nothrow_move_assignable)

// ---------------------------------
// std::has_virtual_destructor
// ---------------------------------

template <class T>
struct has_virtual_destructor : std::bool_constant<__has_virtual_destructor(T)> {
};

template <class T>
constexpr bool has_virtual_destructor_v = has_virtual_destructor<T>::value;

// ------------------------------
// std::is_swappable_with
// ------------------------------

// TODO

// ------------------------------
// std::is_swappable
// ------------------------------

// TODO

// -------------------------------
// std::is_nothrow_swappable
// -------------------------------

// TODO

// ------------------------------------
// std::is_nothrow_swappable_with
// ------------------------------------

// TODO

// ------------------------------
// std::is_destructible
// ------------------------------

// TODO

// ------------------------------------
// std::is_trivially_destructible
// ------------------------------------

// TODO

// ----------------------------------
// std::is_nothrow_destructible
// ----------------------------------

// TODO

}  // namespace std

#endif  // LIBC_INCLUDE_EXTENSIONS_TYPE_TRAITS_HPP_
