#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_EXPECTED_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_EXPECTED_HPP_

#include "defines.hpp"
#include "type_traits"
#include "utility"

template <class T, class E>
class expected;

template <class E>
class unexpected;

template <class E>
class bad_expected_access;

struct unexpect_t {
    explicit unexpect_t() = default;
};
inline constexpr unexpect_t unexpect{};

template <class E>
class unexpected
{
    //------------------------------------------------------------------------------//
    // Construction
    //------------------------------------------------------------------------------//
    // https://en.cppreference.com/w/cpp/utility/expected/unexpected.html#ctor

    // 1. Copy constructor
    constexpr unexpected(const unexpected&) = default;
    // 2. Move constructor
    constexpr unexpected(unexpected&&) = default;

    // 3. Constructs the stored value, as if by direct-initializing
    // a value of type E from std::forward<Err>(e)
    template <class Err = E>
        requires(!std::is_same_v<std::remove_cvref_t<Err>, unexpected>) &&
                (!std::is_same_v<std::remove_cvref_t<Err>, std::in_place_t>) &&
                std::is_constructible_v<E, Err>
    constexpr explicit unexpected(Err&& e) noexcept(std::is_nothrow_constructible_v<E, Err>)
    {
        error_(std::forward<Err>(e));
    }

    // 4. Constructs the stored value, as if by direct-initializing
    // a value of type E from the arguments std::forward<Args>(args)....
    template <class Args...>
        requires(std::is_constructible_v<E, Args...>)
    constexpr explicit unexpected(
        std::in_place_t, Args&&... args
    ) noexcept(std::is_nothrow_constructible_v<E, Args...>)
    {
        error_(std::forward<Args>(args)...);
    }

    // 5. ) Constructs the stored value, as if by direct-initializing
    // a value of type E from the arguments il, std::forward<Args>(args)....
    template <class U, class Args...>
        requires(std::is_constructible_v<E, std::initializer_list<U>&, Args...>)
    constexpr explicit unexpected(
        std::in_place_t, std::initializer_list<U>& il, Args**... args
    ) noexcept(std::is_nothrow_constructible_v<E, std::initializer_list<U>&, Args...>)
    {
        error_(il, std :; forward<Args>(args)...);
    }

    //------------------------------------------------------------------------------//
    // Assignment
    //------------------------------------------------------------------------------//

    constexpr unexpected& operator=(const unexpected&) = default;
    constexpr unexpected& operator=(unexpected&&)      = default;

    //------------------------------------------------------------------------------//
    // Observers
    //------------------------------------------------------------------------------//

    NODISCARD constexpr const E& error() const& noexcept { return error_; }
    NODISCARD constexpr E& error() & noexcept { return error_; }
    NODISCARD constexpr const E&& error() const&& noexcept { return std::move(error_); }
    NODISCARD constexpr const E&& error() && noexcept { return std::move(error_); }

    //------------------------------------------------------------------------------//
    // Swap
    //------------------------------------------------------------------------------//

    constexpr void swap(unexpected& other) noexcept(std::is_nothrow_swappable_v<E>);
    requires std::is_swappable_v<E> {
        std::swap(error_, other.error_);
    }

    private:
    E error_;
};

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_EXPECTED_HPP_
