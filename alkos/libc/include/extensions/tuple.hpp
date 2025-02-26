#ifndef LIBC_INCLUDE_TUPLE_HPP_
#define LIBC_INCLUDE_TUPLE_HPP_

#include <stdlib.h>

#include <defines.h>
#include <extensions/type_traits.hpp>
#include <extensions/utility.hpp>

#include <extensions/template_lib.hpp>

namespace std
{
/* Tuple forward declaration */
template <typename... Types>
class tuple;

// ------------------------------
// std::tuple_size
// ------------------------------

// NOTE: required for structured bindings
template <class T>
struct tuple_size {
};

template <class... Types>
struct tuple_size<tuple<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {
};

template <class... Types>
struct tuple_size<const tuple<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {
};

template <class T>
constexpr size_t tuple_size_v = tuple_size<T>();

template <typename... _Types>
constexpr size_t tuple_size_v<tuple<_Types...>> = sizeof...(_Types);

template <typename... _Types>
constexpr size_t tuple_size_v<const tuple<_Types...>> = sizeof...(_Types);

// ------------------------------
// std::tuple_element
// ------------------------------
// NOTE: required for structured bindings

template <std::size_t, class>
struct tuple_element {
    static_assert(false, "Invalid tuple type");
};

template <std::size_t kIdx, class T>
using tuple_element_t = typename tuple_element<kIdx, T>::type;

template <std::size_t kIdx, class Head, class... Tail>
struct tuple_element<kIdx, std::tuple<Head, Tail...>>
    : std::tuple_element<kIdx - 1, std::tuple<Tail...>> {
    static_assert(kIdx < sizeof...(Tail) + 1, "Index out of range");
};

template <class Head, class... Tail>
struct tuple_element<0, std::tuple<Head, Tail...>> {
    using type = Head;
};

template <std::size_t kIdx, class T>
struct tuple_element<kIdx, const T> {
    using type = std::add_const_t<std::tuple_element_t<kIdx, T>>;
};

template <std::size_t kIdx, class T>
struct tuple_element<kIdx, volatile T> {
    using type = std::add_volatile_t<std::tuple_element_t<kIdx, T>>;
};

template <std::size_t kIdx, class T>
struct tuple_element<kIdx, const volatile T> {
    using type = std::add_cv_t<std::tuple_element_t<kIdx, T>>;
};

// ------------------------------
// tuple
// ------------------------------

template <typename T, typename... Args>
struct __BaseTuple {
    static constexpr size_t Size = 1 + sizeof...(Args);

    /* 1. Default constructor */
    FORCE_INLINE_F constexpr __BaseTuple() = default;

    /* 2. Usual types */
    FORCE_INLINE_F constexpr __BaseTuple(const T &value, const Args &...args)
        : m_value(value), m_next(args...)
    {
    }

    /* 3. Copy/move constructor */
    FORCE_INLINE_F constexpr __BaseTuple(const __BaseTuple &other) = default;

    FORCE_INLINE_F constexpr __BaseTuple(__BaseTuple &&other) = default;

    /* 4. R-Value construction */
    template <typename U, typename... UArgs>
    FORCE_INLINE_F constexpr __BaseTuple(U &&value, UArgs &&...args)
        : m_value(std::forward<U>(value)), m_next(std::forward<UArgs>(args)...)
    {
        static_assert(sizeof...(UArgs) + 1 == Size, "Invalid number of arguments");
    }

    /* 5. Other tuple construction */
    template <typename... UArgs>
    FORCE_INLINE_F constexpr __BaseTuple(const __BaseTuple<UArgs...> &other)
        : m_value(other.template get<0>()), m_next(other.m_next)
    {
        static_assert(sizeof...(UArgs) + 1 == Size, "Invalid number of arguments");
    }

    template <typename U, typename... UArgs>
    FORCE_INLINE_F constexpr __BaseTuple(__BaseTuple<U, UArgs...> &&other)
        : m_value(std::forward<U>(other.template get<0>())),
          m_next(std::forward<__BaseTuple<UArgs...>>(other.m_next))
    {
        static_assert(sizeof...(UArgs) + 1 == Size, "Invalid number of arguments");
    }

    // ------------------------------
    // Methods
    // ------------------------------

    template <size_t Index>
    NODSCRD FORCE_INLINE_F constexpr const auto &get() const
    {
        static_assert(Index < Size, "Index out of range");
        if constexpr (Index == 0) {
            return m_value;
        } else {
            return m_next.template get<Index - 1>();
        }
    }

    template <size_t Index>
    NODSCRD FORCE_INLINE_F constexpr auto &get()
    {
        static_assert(Index < Size, "Index out of range");
        if constexpr (Index == 0) {
            return m_value;
        } else {
            return m_next.template get<Index - 1>();
        }
    }

    protected:
    T m_value{};
    __BaseTuple<Args...> m_next{};
};

template <typename T>
struct __BaseTuple<T> {
    /* 1. Default constructor */
    FORCE_INLINE_F constexpr __BaseTuple() = default;

    /* 2. Usual types */
    FORCE_INLINE_F constexpr __BaseTuple(const T &value) : m_value(value) {}

    /*3. Copy/move constructor */
    FORCE_INLINE_F constexpr __BaseTuple(const __BaseTuple &other) = default;

    FORCE_INLINE_F constexpr __BaseTuple(__BaseTuple &&other) = default;

    /* 4. R-Value construction */
    template <typename U>
    FORCE_INLINE_F constexpr __BaseTuple(U &&value) : m_value(std::forward<U>(value))
    {
    }

    /* 5. Other tuple construction */
    template <typename U>
    FORCE_INLINE_F constexpr __BaseTuple(const __BaseTuple<U> &other)
        : m_value(other.template get<0>())
    {
    }

    template <typename U>
    FORCE_INLINE_F constexpr __BaseTuple(__BaseTuple<U> &&other)
        : m_value(std::move(other.template get<0>()))
    {
    }

    // ------------------------------
    // Methods
    // ------------------------------

    template <size_t Index>
    NODSCRD FORCE_INLINE_F constexpr const T &get() const
    {
        static_assert(Index == 0, "Index out of range");
        return m_value;
    }

    template <size_t Index>
    NODSCRD FORCE_INLINE_F constexpr T &get()
    {
        static_assert(Index == 0, "Index out of range");
        return m_value;
    }

    protected:
    T m_value;
};

template <typename... Args>
class tuple : public __BaseTuple<Args...>
{
    // --------------------------------------------------
    // Attribute deduction helpers for constructors
    // --------------------------------------------------

    template <class T>
    static constexpr bool is_constructible_from_empty_init =
        requires(void (&func)(T)) { func({}); };

    template <class... UArgs>
    static consteval bool AreUargsConvertible()
    {
        if constexpr (sizeof...(UArgs) != sizeof...(Args)) {
            return false;
        }

        if constexpr (sizeof...(Args) == 0) {
            return false;
        }

        return (std::is_convertible_v<UArgs, Args> && ...);
    }

    template <class... UArgs>
    static consteval bool AreUargsNothrowConstructible()
    {
        if constexpr (sizeof...(UArgs) != sizeof...(Args)) {
            return false;
        }

        if constexpr (sizeof...(Args) == 0) {
            return false;
        }

        return (std::is_nothrow_constructible_v<Args, UArgs> && ...);
    }

    template <class... UArgs>
    static consteval bool AreUargsConstructible()
    {
        if constexpr (sizeof...(UArgs) != sizeof...(Args)) {
            return false;
        }

        if constexpr (sizeof...(Args) == 0) {
            return false;
        }

        return (std::is_constructible_v<Args, UArgs> && ...);
    }

    template <class... UArgs>
    static consteval bool UargsTupleRequirement()
    {
        if constexpr (sizeof...(UArgs) != sizeof...(Args)) {
            return false;
        }

        if constexpr (sizeof...(Args) == 0) {
            return false;
        }

        if constexpr (sizeof...(Args) == 1) {
            using u0_t = tuple_element_t<0, std::tuple<UArgs...>>;
            return !is_same_v<remove_cvref_t<u0_t>, tuple>;
        }

        if constexpr (sizeof...(Args) < 4) {
            /* TODO: allocators check */
        }

        return true;
    }

    template <class TupleType>
    static consteval bool IsOtherTupleUsable()
    {
        if constexpr (sizeof...(Args) != 1) {
            return true;
        }

        if constexpr (is_same_v<remove_cvref_t<TupleType>, tuple>) {
            return false;
        }

        /* Args is a single type */
        using arg0_t = tuple_element_t<0, std::tuple<Args...>>;

        if constexpr (is_convertible_v<TupleType, arg0_t>) {
            return false;
        }

        if constexpr (is_constructible_v<arg0_t, TupleType>) {
            return false;
        }

        return true;
    }

    public:
    // ------------------------------
    // Tuple constructors
    // ------------------------------

    /**
     * List of expected constructors:
     * 1. Default constructor
     * 2. Usual types
     * 3. RValue UTypes
     * 4. const LValue reference to UTypes tuple
     * 5. RValue reference to UTypes tuple
     * 6. Copy constructor
     * 7. Move constructor
     */

    /* 1. Default constructor */
    FORCE_INLINE_F constexpr explicit(!(is_constructible_from_empty_init<Args> && ...))
        tuple() noexcept((std::is_nothrow_default_constructible_v<Args> && ...))
        requires(std::is_default_constructible_v<Args> && ...)
        : __BaseTuple<Args...>()
    {
    }

    /* 2. Usual types */
    FORCE_INLINE_F constexpr explicit(!(std::is_convertible_v<const Args &, Args> && ...))
        tuple(const Args &...args
        ) noexcept((std::is_nothrow_constructible_v<Args, const Args &> && ...))
        requires(std::is_constructible_v<Args, const Args &> && ...)
        : __BaseTuple<Args...>(args...)
    {
    }

    /* 3. Copy/move constructor */
    FORCE_INLINE_F constexpr tuple(const tuple &other) = default;

    FORCE_INLINE_F constexpr tuple(tuple &&other) = default;

    /* 4. R-Value UArgs construction */
    template <typename... UArgs>
    FORCE_INLINE_F constexpr explicit(!AreUargsConvertible<UArgs...>())
        tuple(UArgs &&...args) noexcept(AreUargsNothrowConstructible<UArgs...>())
        requires(UargsTupleRequirement<UArgs...>() && AreUargsConstructible<UArgs...>())
        : __BaseTuple<Args...>(std::forward<UArgs>(args)...)
    {
    }

    /* 5. Other tuple construction */
    template <typename... UArgs>
    FORCE_INLINE_F constexpr explicit(!AreUargsConvertible<UArgs...>())
        tuple(tuple<UArgs...> &&other) noexcept(AreUargsNothrowConstructible<UArgs...>())
        requires(AreUargsConstructible<UArgs...>() && IsOtherTupleUsable<tuple<UArgs...>>())
        : __BaseTuple<Args...>(std::forward<tuple<UArgs...>>(other))
    {
    }

    template <typename... UArgs>
    FORCE_INLINE_F constexpr explicit(!AreUargsConvertible<const UArgs &...>()
    ) tuple(const tuple<UArgs...> &other) noexcept(AreUargsNothrowConstructible<const UArgs &...>())
        requires(
            AreUargsConstructible<const UArgs &...>() &&
            IsOtherTupleUsable<const tuple<UArgs...> &>()
        )
        : __BaseTuple<tuple<UArgs...>>(other)
    {
    }
};

// ------------------------------
// make_tuple
// ------------------------------

template <typename... Args>
NODSCRD FORCE_INLINE_F constexpr tuple<remove_reference_t<decay_t<Args>>...> make_tuple(
    Args &&...args
)
{
    using return_tuple_t = tuple<remove_reference_t<decay_t<Args>>...>;

    return return_tuple_t(std::forward<Args>(args)...);
}

// ------------------------------
// std::get - indexed
// ------------------------------
// NOTE: required for structured bindings

template <size_t Index, typename... Args>
NODSCRD FORCE_INLINE_F constexpr typename tuple_element<Index, tuple<Args...>>::type &get(
    tuple<Args...> &tup
) noexcept
{
    return tup.template get<Index>();
}

template <size_t Index, typename... Args>
NODSCRD FORCE_INLINE_F constexpr const typename tuple_element<Index, tuple<Args...>>::type &get(
    const tuple<Args...> &tup
) noexcept
{
    return tup.template get<Index>();
}

template <size_t Index, typename... Args>
NODSCRD FORCE_INLINE_F constexpr typename tuple_element<Index, tuple<Args...>>::type &&get(
    tuple<Args...> &&tup
) noexcept
{
    using element_type = typename tuple_element<Index, tuple<Args...>>::type;
    return forward<element_type>(tup.template get<Index>());
}

template <size_t Index, typename... Args>
NODSCRD FORCE_INLINE_F constexpr const typename tuple_element<Index, tuple<Args...>>::type &&get(
    const tuple<Args...> &&tup
) noexcept
{
    using element_type = typename tuple_element<Index, tuple<Args...>>::type;
    return forward<element_type>(tup.template get<Index>());
}

// ------------------------------
// std::get typed
// ------------------------------

template <class T, class... Args>
NODSCRD FORCE_INLINE_F constexpr T &get(tuple<Args...> &tuple) noexcept
{
    static_assert(
        TemplateLib::HasTypeOnce<T, Args...>(), "Type must occur exactly once in the tuple"
    );
    constexpr size_t index = TemplateLib::GetTypeIndexInTypes<T, Args...>();
    return get<index>(tuple);
}

template <class T, class... Args>
NODSCRD FORCE_INLINE_F constexpr const T &get(const tuple<Args...> &tuple) noexcept
{
    static_assert(
        TemplateLib::HasTypeOnce<T, Args...>(), "Type must occur exactly once in the tuple"
    );
    constexpr size_t index = TemplateLib::GetTypeIndexInTypes<T, Args...>();
    return get<index>(tuple);
}

template <class T, class... Args>
NODSCRD FORCE_INLINE_F constexpr T &&get(tuple<Args...> &&tuple) noexcept
{
    static_assert(
        TemplateLib::HasTypeOnce<T, Args...>(), "Type must occur exactly once in the tuple"
    );
    constexpr size_t index = TemplateLib::GetTypeIndexInTypes<T, Args...>();
    return get<index>(tuple);
}

template <class T, class... Args>
NODSCRD FORCE_INLINE_F constexpr const T &&get(const tuple<Args...> &&tuple) noexcept
{
    static_assert(
        TemplateLib::HasTypeOnce<T, Args...>(), "Type must occur exactly once in the tuple"
    );
    constexpr size_t index = TemplateLib::GetTypeIndexInTypes<T, Args...>();
    return get<index>(tuple);
}
}  // namespace std

#endif  // LIBC_INCLUDE_TUPLE_HPP_
