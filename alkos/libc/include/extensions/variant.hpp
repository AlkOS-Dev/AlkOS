#ifndef ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_VARIANT_HPP_
#define ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_VARIANT_HPP_

#include <assert.h>
#include <extensions/cstddef.hpp>
#include <extensions/internal/aligned_membuffer.hpp>
#include <extensions/memory.hpp>
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
// Forward declarations of get / get_if
//------------------------------------------------------------------------------//

// Forward declarations for get<kIndex>
template <size_t kIndex, typename... Types>
constexpr variant_alternative_t<kIndex, variant<Types...>>& get(variant<Types...>& variant_v);
template <size_t kIndex, typename... Types>
constexpr variant_alternative_t<kIndex, variant<Types...>>&& get(variant<Types...>&& variant_v);
template <size_t kIndex, typename... Types>
constexpr const variant_alternative_t<kIndex, variant<Types...>>& get(
    const variant<Types...>& variant_v
);
template <size_t kIndex, typename... Types>
constexpr const variant_alternative_t<kIndex, variant<Types...>>&& get(
    const variant<Types...>&& variant_v
);

// Forward declarations for get<T>
template <typename T, typename... Types>
constexpr T& get(variant<Types...>& variant_v);
template <typename T, typename... Types>
constexpr T&& get(variant<Types...>&& variant_v);
template <typename T, typename... Types>
constexpr const T& get(const variant<Types...>& variant_v);
template <typename T, typename... Types>
constexpr const T&& get(const variant<Types...>&& variant_v);

// Forward declarations for get_if<kIndex>
template <size_t kIndex, typename... Types>
constexpr add_pointer_t<variant_alternative_t<kIndex, variant<Types...>>> get_if(
    variant<Types...>* variant_v
) noexcept;
template <size_t kIndex, typename... Types>
constexpr add_pointer_t<const variant_alternative_t<kIndex, variant<Types...>>> get_if(
    const variant<Types...>* variant_v
) noexcept;

// Forward declarations for get_if<T>
template <typename T, typename... Types>
constexpr add_pointer_t<T> get_if(variant<Types...>* variant_v) noexcept;
template <typename T, typename... Types>
constexpr add_pointer_t<const T> get_if(const variant<Types...>* variant_v) noexcept;

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
            return *mem_buffer_first.Get();
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

template <bool IsTrivialyDestructible, typename... Types>
struct VariantStorage;

template <typename... Types>
struct VariantStorage<false, Types...> {
    protected:
    //------------------------------------------------------------------------------//
    // Internal Types
    //------------------------------------------------------------------------------//

    public:
    using index_t = std::size_t;  // TODO: Could implement a getter for the smallest viable type

    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//
    constexpr VariantStorage() noexcept : index_(variant_npos) {}

    constexpr void Reset() noexcept
    {
        // TODO
        index_ = index_t(variant_npos);
    }

    ~VariantStorage() { Reset(); }

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    void* GetPtr() const noexcept
    {
        return const_cast<void*>(static_cast<const void*>(std::addressof(storage_)));
    }

    constexpr index_t GetIndex() const noexcept { return index_; }

    constexpr bool IsIndexNpos() const noexcept { return index_ == index_t(variant_npos); }

    constexpr bool IsValid() const noexcept
    {
        // TODO
        return !IsIndexNpos();
    }

    protected:
    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//
    VariadicUnion<Types...> storage_;
    index_t index_;
};

// TODO: VariantStorage<true, Types...>

}  // namespace variant
}  // namespace detail

template <typename... Types>
class variant : protected detail::variant::VariantStorage<
                    detail::variant::Traits<Types...>::has_trivial_destructor, Types...>
{
    STATIC_ASSERT(sizeof...(Types) > 0, "Variant must have at least one type");
    STATIC_ASSERT((!is_array_v<Types> && ...), "Arrays are not allowed");
    STATIC_ASSERT((!is_reference_v<Types> && ...), "References are not allowed");
    STATIC_ASSERT((!is_void_v<Types> && ...), "Void is not allowed");

    private:
    //------------------------------------------------------------------------------//
    // Internal Types
    //------------------------------------------------------------------------------//
    using Traits = detail::variant::Traits<Types...>;
    using VariantStorage =
        detail::variant::VariantStorage<Traits::has_trivial_destructor, Types...>;
    using index_t = std::size_t;

    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    /* 1. Default constructor */
    constexpr variant() noexcept(Traits::has_nothrow_default_constructor)
        requires(Traits::has_default_constructor)
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
        return !VariantStorage::IsValid();
    }

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//
    constexpr size_t index()
    {
        if (VariantStorage::IsIndexNpos()) {
            return variant_npos;
        }
        return VariantStorage::GetIndex();
    }

    private:
    //------------------------------------------------------------------------------//
    // Private Methods
    //------------------------------------------------------------------------------//

    constexpr void initialize_valueless() noexcept { VariantStorage::Reset(); }

    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//

    //------------------------------------------------------------------------------//
    // Helpers
    //------------------------------------------------------------------------------//

    template <size_t kIndex>
    constexpr auto& _get_internal() & noexcept
    {
        // Access the storage_ union directly
        return this->storage_.template get<kIndex>();
    }

    template <size_t kIndex>
    constexpr const auto& _get_internal() const& noexcept
    {
        return this->storage_.template get<kIndex>();
    }

    template <size_t kIndex>
    constexpr auto&& _get_internal() && noexcept
    {
        return std::move(this->storage_.template get<kIndex>());
    }

    template <size_t kIndex>
    constexpr const auto&& _get_internal() const&& noexcept
    {
        return std::move(this->storage_.template get<kIndex>());
    }

    //------------------------------------------------------------------------------//
    // Friends
    //------------------------------------------------------------------------------//

    // Friends for get<kIndex>
    template <size_t kOtherIndex, typename... OtherTypes>
    friend constexpr variant_alternative_t<kOtherIndex, variant<OtherTypes...>>& get(
        variant<OtherTypes...>& variant_v
    );

    template <size_t kOtherIndex, typename... OtherTypes>
    friend constexpr variant_alternative_t<kOtherIndex, variant<OtherTypes...>>&& get(
        variant<OtherTypes...>&& variant_v
    );

    template <size_t kOtherIndex, typename... OtherTypes>
    friend constexpr const variant_alternative_t<kOtherIndex, variant<OtherTypes...>>& get(
        const variant<OtherTypes...>& variant_v
    );

    template <size_t kOtherIndex, typename... OtherTypes>
    friend constexpr const variant_alternative_t<kOtherIndex, variant<OtherTypes...>>&& get(
        const variant<OtherTypes...>&& variant_v
    );

    // Friends for get<T>
    template <typename TOther, typename... OtherTypes>
    friend constexpr TOther& get(variant<OtherTypes...>& variant_v);

    template <typename TOther, typename... OtherTypes>
    friend constexpr TOther&& get(variant<OtherTypes...>&& variant_v);

    template <typename TOther, typename... OtherTypes>
    friend constexpr const TOther& get(const variant<OtherTypes...>& variant_v);

    template <typename TOther, typename... OtherTypes>
    friend constexpr const TOther&& get(const variant<OtherTypes...>&& variant_v);

    // Friends for get_if<kIndex>
    template <size_t kOtherIndex, typename... OtherTypes>
    friend constexpr add_pointer_t<variant_alternative_t<kOtherIndex, variant<OtherTypes...>>>
    get_if(variant<OtherTypes...>* variant_v) noexcept;

    template <size_t kOtherIndex, typename... OtherTypes>
    friend constexpr add_pointer_t<const variant_alternative_t<kOtherIndex, variant<OtherTypes...>>>
    get_if(const variant<OtherTypes...>* variant_v) noexcept;

    // Friends for get_if<T>
    template <typename TOther, typename... OtherTypes>
    friend constexpr add_pointer_t<TOther> get_if(variant<OtherTypes...>* variant_v) noexcept;

    template <typename TOther, typename... OtherTypes>
    friend constexpr add_pointer_t<const TOther> get_if(const variant<OtherTypes...>* variant_v
    ) noexcept;
};

//------------------------------------------------------------------------------//
// get
//------------------------------------------------------------------------------//

// By index

template <size_t kIndex, typename... Types>
constexpr variant_alternative<kIndex, variant<Types...>>& get(variant<Types...>& variant_v)
{
    STATIC_ASSERT(
        (kIndex < sizeof...(Types)), "Index should be less than the number of types in the variant"
    );
    if (variant_v.index() != kIndex) {
        throw_bad_variant_access();
    }
    return variant_v.template _get_internal<kIndex>();
}

template <size_t kIndex, typename... Types>
constexpr variant_alternative_t<kIndex, variant<Types...>>&& get(variant<Types...>&& variant_v)
{
    STATIC_ASSERT(
        (kIndex < sizeof...(Types)), "Index should be less than the number of types in the variant"
    );
    if (variant_v.index() != kIndex) {
        throw_bad_variant_access();
    }
    return variant_v.template _get_internal<kIndex>();
}

template <size_t kIndex, typename... Types>
constexpr const variant_alternative_t<kIndex, variant<Types...>>& get(
    const variant<Types...>& variant_v
)
{
    STATIC_ASSERT(
        (kIndex < sizeof...(Types)), "Index should be less than the number of types in the variant"
    );
    if (variant_v.index() != kIndex) {
        throw_bad_variant_access();
    }
    return variant_v.template _get_internal<kIndex>();
}

template <size_t kIndex, typename... Types>
constexpr const variant_alternative_t<kIndex, variant<Types...>>&& get(
    const variant<Types...>&& variant_v
)
{
    STATIC_ASSERT(
        (kIndex < sizeof...(Types)), "Index should be less than the number of types in the variant"
    );
    if (variant_v.index() != kIndex) {
        throw_bad_variant_access();
    }
    return variant_v.template _get_internal<kIndex>();
}

// By type

template <typename T, typename... Types>
constexpr T& get(variant<Types...>& variant_v)
{
    STATIC_ASSERT(
        (detail::variant::index_of_v<T, Types...> != sizeof...(Types)),
        "Type should be in the variant"
    );
    return get<detail::variant::index_of_v<T, Types...>>(variant_v);
}

template <typename T, typename... Types>
constexpr T&& get(variant<Types...>&& variant_v)
{
    STATIC_ASSERT(
        (detail::variant::index_of_v<T, Types...> != sizeof...(Types)),
        "Type should be in the variant"
    );
    return get<detail::variant::index_of_v<T, Types...>>(std::move(variant_v));
}

template <typename T, typename... Types>
constexpr const T& get(const variant<Types...>& variant_v)
{
    STATIC_ASSERT(
        (detail::variant::index_of_v<T, Types...> != sizeof...(Types)),
        "Type should be in the variant"
    );
    return get<detail::variant::index_of_v<T, Types...>>(variant_v);
}

template <typename T, typename... Types>
constexpr const T&& get(const variant<Types...>&& variant_v)
{
    STATIC_ASSERT(
        (detail::variant::index_of_v<T, Types...> != sizeof...(Types)),
        "Type should be in the variant"
    );
    return get<detail::variant::index_of_v<T, Types...>>(std::move(variant_v));
}

// get_if

// by index

template <size_t kIndex, typename... Types>
constexpr add_pointer_t<variant_alternative_t<kIndex, variant<Types...>>> get_if(
    variant<Types...>* variant_v
) noexcept
{
    STATIC_ASSERT(
        (kIndex < sizeof...(Types)), "Index should be less than the number of types in the variant"
    );

    if (variant_v == nullptr || variant_v->index() != kIndex) {
        return nullptr;
    }
    return std::addressof(variant_v->template _get_internal<kIndex>());
}

template <size_t kIndex, typename... Types>
constexpr add_pointer_t<const variant_alternative_t<kIndex, variant<Types...>>> get_if(
    const variant<Types...>* variant_v
) noexcept
{
    STATIC_ASSERT(
        (kIndex < sizeof...(Types)), "Index should be less than the number of types in the variant"
    );

    if (variant_v == nullptr || variant_v->index() != kIndex) {
        return nullptr;
    }
    return std::addressof(variant_v->template _get_internal<kIndex>());
}

// by type

template <typename T, typename... Types>
constexpr add_pointer_t<T> get_if(variant<Types...>* variant_v) noexcept
{
    STATIC_ASSERT(
        (detail::variant::index_of_v<T, Types...> != sizeof...(Types)),
        "Type should be in the variant"
    );

    constexpr size_t kIndex = detail::variant::index_of_v<T, Types...>;
    return get_if<kIndex>(variant_v);
}

template <typename T, typename... Types>
constexpr add_pointer_t<const T> get_if(const variant<Types...>* variant_v) noexcept
{
    STATIC_ASSERT(
        (detail::variant::index_of_v<T, Types...> != sizeof...(Types)),
        "Type should be in the variant"
    );

    constexpr size_t kIndex = detail::variant::index_of_v<T, Types...>;
    return get_if<kIndex>(variant_v);
}

}  // namespace std
#endif  // ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_VARIANT_HPP_
