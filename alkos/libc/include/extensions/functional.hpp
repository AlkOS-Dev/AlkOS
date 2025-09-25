#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_FUNCTIONAL_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_FUNCTIONAL_HPP_

#include <extensions/internal/invoke.hpp>
#include "utility.hpp"

namespace std
{
// ------------------------------
// std::reference_wrapper
// ------------------------------

// TODO

// ------------------------------
// std::ref
// ------------------------------

// TODO

// ------------------------------
// std::cref
// ------------------------------

//------------------------------------------------------------------------------
// std::invoke and std::invoke_r
//------------------------------------------------------------------------------
// TODO: I needed this quickly so it's actually copied from
// https://en.cppreference.com/w/cpp/utility/functional/invoke
// Should be cleaned up
//------------------------------------------------------------------------------

namespace detail
{
template <class>
constexpr bool is_reference_wrapper_v = false;
template <class U>
constexpr bool is_reference_wrapper_v<std::reference_wrapper<U>> = true;

template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <class C, class Pointed, class Object, class... Args>
constexpr decltype(auto) invoke_memptr(Pointed C::* member, Object&& object, Args&&... args)
{
    using object_t                    = remove_cvref_t<Object>;
    constexpr bool is_member_function = std::is_function_v<Pointed>;
    constexpr bool is_wrapped         = is_reference_wrapper_v<object_t>;
    constexpr bool is_derived_object =
        std::is_same_v<C, object_t> || std::is_base_of_v<C, object_t>;

    if constexpr (is_member_function) {
        if constexpr (is_derived_object)
            return (std::forward<Object>(object).*member)(std::forward<Args>(args)...);
        else if constexpr (is_wrapped)
            return (object.get().*member)(std::forward<Args>(args)...);
        else
            return ((*std::forward<Object>(object)).*member)(std::forward<Args>(args)...);
    } else {
        static_assert(std::is_object_v<Pointed> && sizeof...(args) == 0);
        if constexpr (is_derived_object)
            return std::forward<Object>(object).*member;
        else if constexpr (is_wrapped)
            return object.get().*member;
        else
            return (*std::forward<Object>(object)).*member;
    }
}
}  // namespace detail

template <class F, class... Args>
constexpr std::invoke_result_t<F, Args...> invoke(
    F&& f, Args&&... args
) noexcept(std::is_nothrow_invocable_v<F, Args...>)
{
    if constexpr (std::is_member_pointer_v<detail::remove_cvref_t<F>>)
        return detail::invoke_memptr(f, std::forward<Args>(args)...);
    else
        return std::forward<F>(f)(std::forward<Args>(args)...);
}

template <class R, class F, class... Args>
    requires std::is_invocable_r_v<R, F, Args...>
constexpr R invoke_r(F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_r_v<R, F, Args...>)
{
    if constexpr (std::is_void_v<R>)
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    else
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_FUNCTIONAL_HPP_
