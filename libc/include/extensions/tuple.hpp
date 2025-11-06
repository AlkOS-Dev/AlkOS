#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_TUPLE_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_TUPLE_HPP_

#include <stdlib.h>

#include <defines.h>
#include <extensions/type_traits.hpp>
#include <extensions/utility.hpp>

#include <extensions/internal/tuple_base.hpp>
#include <extensions/template_lib.hpp>

namespace std
{
// ------------------------------
// make_tuple
// ------------------------------

template <typename... Args>
NODISCARD FORCE_INLINE_F constexpr tuple<remove_reference_t<decay_t<Args>>...> make_tuple(
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
NODISCARD FORCE_INLINE_F constexpr typename tuple_element<Index, tuple<Args...>>::type &get(
    tuple<Args...> &tup
) noexcept
{
    return tup.template get<Index>();
}

template <size_t Index, typename... Args>
NODISCARD FORCE_INLINE_F constexpr const typename tuple_element<Index, tuple<Args...>>::type &get(
    const tuple<Args...> &tup
) noexcept
{
    return tup.template get<Index>();
}

template <size_t Index, typename... Args>
NODISCARD FORCE_INLINE_F constexpr typename tuple_element<Index, tuple<Args...>>::type &&get(
    tuple<Args...> &&tup
) noexcept
{
    using element_type = typename tuple_element<Index, tuple<Args...>>::type;
    return forward<element_type>(tup.template get<Index>());
}

template <size_t Index, typename... Args>
NODISCARD FORCE_INLINE_F constexpr const typename tuple_element<Index, tuple<Args...>>::type &&get(
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
NODISCARD FORCE_INLINE_F constexpr T &get(tuple<Args...> &tuple) noexcept
{
    static_assert(
        template_lib::HasTypeOnce<T, Args...>(), "Type must occur exactly once in the tuple"
    );
    constexpr size_t index = template_lib::GetTypeIndexInTypes<T, Args...>();
    return get<index>(tuple);
}

template <class T, class... Args>
NODISCARD FORCE_INLINE_F constexpr const T &get(const tuple<Args...> &tuple) noexcept
{
    static_assert(
        template_lib::HasTypeOnce<T, Args...>(), "Type must occur exactly once in the tuple"
    );
    constexpr size_t index = template_lib::GetTypeIndexInTypes<T, Args...>();
    return get<index>(tuple);
}

template <class T, class... Args>
NODISCARD FORCE_INLINE_F constexpr T &&get(tuple<Args...> &&tuple) noexcept
{
    static_assert(
        template_lib::HasTypeOnce<T, Args...>(), "Type must occur exactly once in the tuple"
    );
    constexpr size_t index = template_lib::GetTypeIndexInTypes<T, Args...>();
    return get<index>(tuple);
}

template <class T, class... Args>
NODISCARD FORCE_INLINE_F constexpr const T &&get(const tuple<Args...> &&tuple) noexcept
{
    static_assert(
        template_lib::HasTypeOnce<T, Args...>(), "Type must occur exactly once in the tuple"
    );
    constexpr size_t index = template_lib::GetTypeIndexInTypes<T, Args...>();
    return get<index>(tuple);
}
}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_TUPLE_HPP_
