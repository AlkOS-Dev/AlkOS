#ifndef ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_VARIANT_HPP_
#define ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_VARIANT_HPP_

#include <assert.h>
#include <extensions/cstddef.hpp>
#include <extensions/type_traits.hpp>

namespace std
{

// References:
// https://github.com/gcc-mirror/gcc/tree/master/libstdc%2B%2B-v3
// https://github.com/llvm/llvm-project/blob/main/libcxx/include/variant

namespace detail
{
namespace variant
{

template <typename... Types>
struct variant;
template <typename... Types>
struct tuple;

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
// Special value to indicate that an index is not in the variant
//------------------------------------------------------------------------------//

inline constexpr size_t variant_npos = -1;

//------------------------------------------------------------------------------//
// index_of
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
}  // namespace variant
}  // namespace detail

template <typename... Types>
class variant
{
    STATIC_ASSERT(sizeof...(Types) > 0, "Variant must have at least one type");

    private:
    //------------------------------------------------------------------------------//
    // Internal Types
    //------------------------------------------------------------------------------//
    using storage_t = std::aligned_union<0, Types...>;

    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//

    private:
    //------------------------------------------------------------------------------//
    // Private Methods
    //------------------------------------------------------------------------------//

    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//

    //------------------------------------------------------------------------------//
    // Helpers
    //------------------------------------------------------------------------------//
};

}  // namespace std
#endif  // ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_VARIANT_HPP_
