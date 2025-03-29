#ifndef ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_VARIANT_HPP_
#define ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_VARIANT_HPP_

#include <assert.h>
#include <extensions/cstddef.hpp>
#include <extensions/internal/aligned_membuffer.hpp>
#include <extensions/template_lib.hpp>
#include <extensions/type_traits.hpp>
#include <extensions/utility.hpp>

namespace std
{

// References:
// https://github.com/gcc-mirror/gcc/tree/master/libstdc%2B%2B-v3
// https://github.com/llvm/llvm-project/blob/main/libcxx/include/variant

template <typename... Types>
class variant;
template <typename... Types>
class tuple;

namespace detail
{
namespace variant
{
}  // namespace variant
}  // namespace detail

NO_RET FAST_CALL void throw_bad_variant_access()
{
    if constexpr (kIsKernelBuild) {
        FAIL_ALWAYS("Bad variant access");
    } else {
        TODO_USERSPACE_EXCEPTIONS
        FAIL_ALWAYS("Bad variant access");
    }
}

//------------------------------------------------------------------------------//
// variant_size
//------------------------------------------------------------------------------//
// Returns the number of alternative types in Variant
//------------------------------------------------------------------------------//

template <class Variant>
struct variant_size;

template <class Variant>
struct variant_size<const Variant> : variant_size<Variant> {
};

template <class Variant>
struct variant_size<volatile Variant> : variant_size<Variant> {
};

template <class Variant>
struct variant_size<const volatile Variant> : variant_size<Variant> {
};

template <class... Types>
struct variant_size<variant<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {
};

template <class Variant>
inline constexpr size_t variant_size_v = variant_size<Variant>::value;

//------------------------------------------------------------------------------//
// variant_alternative
//------------------------------------------------------------------------------//
// Returns the type of the alternative at Index in Variant
//------------------------------------------------------------------------------//

template <size_t Index, class Variant>
struct variant_alternative;

template <size_t Index, typename First, typename... Rest>
struct variant_alternative<Index, variant<First, Rest...>>
    : variant_alternative<Index, variant<Rest...>> {
};

template <typename First, typename... Rest>
struct variant_alternative<0, variant<First, Rest...>> {
    using type = First;
};

template <size_t Index, class Variant>
using variant_alternative_t = typename variant_alternative<Index, Variant>::type;

template <size_t Index, typename Variant>
struct variant_alternative<Index, const Variant> {
    using type = add_const_t<variant_alternative_t<Index, Variant>>;
};
template <size_t Index, typename Variant>
struct variant_alternative<Index, volatile Variant> {
    using type = add_volatile_t<variant_alternative_t<Index, Variant>>;
};
template <size_t Index, typename Variant>
struct variant_alternative<Index, const volatile Variant> {
    using type = add_cv_t<variant_alternative_t<Index, Variant>>;
};

//------------------------------------------------------------------------------//
// variant_npos
//------------------------------------------------------------------------------//
// Special value to indicate that an index is not in the variant
//------------------------------------------------------------------------------//

inline constexpr size_t variant_npos = -1;

namespace detail
{
namespace variant
{
//------------------------------------------------------------------------------//
// index_of
//------------------------------------------------------------------------------//
// Returns the index of the first occurrence of Type in Types
// If Type is not in Types, returns sizeof...(Types)
//------------------------------------------------------------------------------//
template <typename Type, typename... Types>
struct index_of : std::integral_constant<size_t, 0> {
};

template <typename Type, typename... Types>
inline constexpr size_t index_of_v = index_of<Type, Types...>::value;

template <typename Type, typename First, typename... Rest>
struct index_of<Type, First, Rest...>
    : std::integral_constant<size_t, is_same_v<Type, First> ? 0 : index_of_v<Type, Rest...> + 1> {
};

//------------------------------------------------------------------------------//
// VariadicUnion
//------------------------------------------------------------------------------//
// A union that can hold any of the types in the parameter pack
//------------------------------------------------------------------------------//

template <typename... Types>
union VariadicUnion {
};

template <typename First, typename... Rest>
union VariadicUnion<First, Rest...> {
    private:
    //------------------------------------------------------------------------------//
    // Internal Types
    //------------------------------------------------------------------------------//

    public:
    //------------------------------------------------------------------------------//
    // Creation and Destruction
    //------------------------------------------------------------------------------//

    /* 1. Default constructor */
    template <typename... Args>
    constexpr VariadicUnion() : rest()
    {
    }

    /* 2. Constructor of Nth type */
    template <typename... Args>
    constexpr VariadicUnion(std::in_place_index_t<0>, Args&&... args)
    {
        new (mem_buffer_first.GetAddress()) First(std::forward<Args>(args)...);
    }

    template <size_t kIndex, typename... Args>
    constexpr VariadicUnion(std::in_place_index_t<kIndex>, Args&&... args)
        : rest(in_place_index<kIndex - 1>, std::forward<Args>(args)...)
    {
    }

    //------------------------------------------------------------------------------//
    // Methods
    //------------------------------------------------------------------------------//
    template <size_t kIndex>
    constexpr decltype(auto) get(in_place_index_t<kIndex>) noexcept
    {
        STATIC_ASSERT((kIndex < 1 + sizeof...(Rest)), "Index out of range");
        if constexpr (kIndex == 0) {
            return *mem_buffer_first.GetPtr();
        } else {
            return rest.get(in_place_index_t<kIndex - 1>{});
        }
    }

    template <size_t kIndex>
    constexpr decltype(auto) get() const noexcept
    {
        return get<kIndex>(in_place_index_t<kIndex>{});
    }

    //------------------------------------------------------------------------------//
    // Fields
    //------------------------------------------------------------------------------//
    ::internal::AlignedMemoryBuffer<First> mem_buffer_first;
    VariadicUnion<Rest...> rest;

    //------------------------------------------------------------------------------//
    // Helpers
    //------------------------------------------------------------------------------//
};

//------------------------------------------------------------------------------//
// Traits
//------------------------------------------------------------------------------//
// Contains the meta information about the variant such as if it has a default
// constructor is the constructor trivial, etc. and exposes them as constexpr
// values
//------------------------------------------------------------------------------//

// TODO: Might change the "&&" to "||" depending on template parameter, and move this to TemplateLib
template <typename... Types>
struct Traits {
    static constexpr bool has_default_constructor =
        is_default_constructible_v<typename TemplateLib::nth_type_t<0, Types...>>;
    static constexpr bool has_copy_constructor   = (is_copy_constructible_v<Types> && ...);
    static constexpr bool has_move_constructor   = (is_move_constructible_v<Types> && ...);
    static constexpr bool has_copy_assign        = (is_copy_assignable_v<Types> && ...);
    static constexpr bool has_move_assign        = (is_move_assignable_v<Types> && ...);
    static constexpr bool has_trivial_destructor = (is_trivially_constructible_v<Types> && ...);
    static constexpr bool has_trivial_copy_constructor =
        (is_trivially_copy_constructible_v<Types> && ...);
    static constexpr bool has_trivial_move_constructor =
        (is_trivially_move_constructible_v<Types> && ...);
    static constexpr bool has_trivial_copy_assign = (is_trivially_copy_assignable_v<Types> && ...);
    static constexpr bool has_trivial_move_assign = (is_trivially_move_assignable_v<Types> && ...);
    static constexpr bool has_nothrow_default_constructor =
        is_nothrow_constructible_v<typename TemplateLib::nth_type_t<0, Types...>>;
    static constexpr bool has_nothrow_copy_constructor =
        (is_nothrow_copy_constructible_v<Types> && ...);
    static constexpr bool has_nothrow_move_constructor =
        (is_nothrow_move_constructible_v<Types> && ...);
    static constexpr bool has_nothrow_copy_assign = (is_nothrow_copy_assignable_v<Types> && ...);
    static constexpr bool has_nothrow_move_assign = (is_nothrow_move_assignable_v<Types> && ...);
};

//------------------------------------------------------------------------------//
// get
//------------------------------------------------------------------------------//
// An internal helper function
//------------------------------------------------------------------------------//

}  // namespace variant
}  // namespace detail

template <typename... Types>
class variant
{
    STATIC_ASSERT(sizeof...(Types) > 0, "Variant must have at least one type");
    STATIC_ASSERT((!is_array_v<Types> && ...), "Arrays are not allowed");
    STATIC_ASSERT((!is_reference_v<Types> && ...), "References are not allowed");
    STATIC_ASSERT((!is_void_v<Types> && ...), "Void is not allowed");

    private:
    //------------------------------------------------------------------------------//
    // Internal Types
    //------------------------------------------------------------------------------//
    using storage_t = std::aligned_union<0, Types...>;
    using Traits    = detail::variant::Traits<Types...>;
    using index_t   = std::size_t;  // TODO: Could implement a getter for the smallest viable type

    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    /* 1. Default constructor */
    constexpr variant() noexcept(Traits::has_nothrow_default_constructor)
        requires(Traits::has_default_constructor)
        : index_(variant_npos)
    {
        // TODO
    }

    /* 2. Copy constructor */
    constexpr variant(const variant& other) noexcept(Traits::has_nothrow_copy_constructor)
        requires(Traits::has_copy_constructor)
    {
        if constexpr (other.valueless_by_exception()) {
            initialize_valueless();
            return;
        }

        // TODO
    }

    /* 3. Move constructor */
    constexpr variant(variant&& other) noexcept(Traits::has_nothrow_move_constructor)
        requires(Traits::has_move_constructor)
    {
        if constexpr (other.valueless_by_exception()) {
            initialize_valueless();
            return;
        }
        // TODO
    }

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//
    FORCE_INLINE_F constexpr bool valueless_by_exception() const noexcept
    {
        return index_ == variant_npos;
    }

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//
    constexpr index_t index() { return index_; }

    private:
    //------------------------------------------------------------------------------//
    // Private Methods
    //------------------------------------------------------------------------------//

    template <size_t I>
    constexpr TemplateLib::nth_type_t<I, Types...> get_ptr() noexcept
    {
        return &storage_;
    }

    constexpr void initialize_valueless() noexcept { index_ = variant_npos; }

    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//
    storage_t storage_;
    index_t index_ = variant_npos;

    //------------------------------------------------------------------------------//
    // Helpers
    //------------------------------------------------------------------------------//
};

//------------------------------------------------------------------------------//
// get
//------------------------------------------------------------------------------//

// Specialization where we use the index to get the value
template <size_t I, typename... Types>
constexpr variant_alternative<I, Types...>& get(variant<Types...>& variant_v)
{
    STATIC_ASSERT(
        I < sizeof...(Types), "Index should be less than the number of types in the variant"
    );
    if (variant_v.index() != I) {
        throw_bad_variant_access();
    }
}

template <typename T, typename... Types>
constexpr T& get(variant<Types...>& variant_v)
{
    STATIC_ASSERT(
        (TemplateLib::HasTypeOnce<T, Types...>()), "Type must occur exactly once in the variant"
    );
    STATIC_ASSERT((!std::is_void_v<T>), "Void is not allowed");

    return std::get<detail::variant::index_of_v<T, Types...>>(variant_v);
}

}  // namespace std
#endif  // ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_VARIANT_HPP_
