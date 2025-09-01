#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_EXPECTED_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_EXPECTED_HPP_

#include "assert.hpp"
#include "defines.hpp"
#include "memory.hpp"
#include "type_traits.hpp"
#include "utility.hpp"

#define BAD_EXPECTED_ACCESS(msg) R_FAIL_ALWAYS(msg)

namespace std
{

template <class T, class E>
class expected;

template <class E>
class unexpected;

struct unexpect_t {
    explicit unexpect_t() = default;
};
inline constexpr unexpect_t unexpect{};

//------------------------------------------------------------------------------//
// Class : Unexpected
//------------------------------------------------------------------------------//

template <class E>
class unexpected
{
    static_assert(
        is_object_v<E> && !is_array_v<E> && !is_const_v<E> && !is_volatile_v<E>,
        "std::unexpected error type E must be a non-const, non-volatile object type that is not an "
        "array."
    );

    public:
    //------------------------------------------------------------------------------//
    // Construction
    //------------------------------------------------------------------------------//
    // https://en.cppreference.com/w/cpp/utility/expected/unexpected.html#ctor

    // 1. Copy constructor
    constexpr unexpected(const unexpected&) = default;
    // 2. Move constructor
    constexpr unexpected(unexpected&&) = default;

    // 3. Constructs the stored value, as if by direct-initializing
    // a value of type E from forward<Err>(e)
    template <class Err = E>
        requires(
            !is_same_v<remove_cvref_t<Err>, unexpected> &&
            !is_same_v<remove_cvref_t<Err>, in_place_t> && is_constructible_v<E, Err>
        )
    constexpr explicit unexpected(Err&& e) noexcept(is_nothrow_constructible_v<E, Err>)
        : error_(forward<Err>(e))
    {
    }

    // 4. Constructs the stored value, as if by direct-initializing
    // a value of type E from the arguments forward<Args>(args)....
    template <class... Args>
        requires(is_constructible_v<E, Args...>)
    constexpr explicit unexpected(
        in_place_t, Args&&... args
    ) noexcept(is_nothrow_constructible_v<E, Args...>)
        : error_(forward<Args>(args)...)
    {
    }

    // 5. ) Constructs the stored value, as if by direct-initializing
    // a value of type E from the arguments il, forward<Args>(args)....
    template <class U, class... Args>
        requires(is_constructible_v<E, initializer_list<U>&, Args...>)
    constexpr explicit unexpected(
        in_place_t, initializer_list<U> il, Args&&... args
    ) noexcept(is_nothrow_constructible_v<E, initializer_list<U>&, Args...>)
        : error_(il, std::forward<Args>(args)...)
    {
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
    NODISCARD constexpr const E&& error() const&& noexcept { return move(error_); }
    NODISCARD constexpr E&& error() && noexcept { return move(error_); }

    //------------------------------------------------------------------------------//
    // Swap
    //------------------------------------------------------------------------------//

    constexpr void swap(unexpected& other) noexcept(is_nothrow_swappable_v<E>)
        requires(is_swappable_v<E>)
    {
        using std::swap;
        swap(error_, other.error_);
    }

    friend constexpr void swap(unexpected& x, unexpected& y) noexcept(noexcept(x.swap(y)))
        requires(is_swappable_v<E>)
    {
        x.swap(y);
    }

    //------------------------------------------------------------------------------//
    // Comparasion
    //------------------------------------------------------------------------------//

    template <class OtherE>
    NODISCARD friend constexpr bool operator==(const unexpected& x, const unexpected<OtherE>& y)
    {
        return x.error() == y.error();
    }

    //------------------------------------------------------------------------------//
    // Data
    //------------------------------------------------------------------------------//

    private:
    E error_;
};

template <typename E>
unexpected(E) -> unexpected<E>;

//------------------------------------------------------------------------------//
// Class : Expected
//------------------------------------------------------------------------------//

namespace detail
{
template <class T>
constexpr bool is_unexpected_v = false;
template <class E>
constexpr bool is_unexpected_v<unexpected<E>> = true;
}  // namespace detail

template <class T, class E>
class expected
{
    static_assert(!is_reference_v<T>, "T must not be a reference type");
    static_assert(!is_function_v<T>, "T must not be a function type");
    static_assert(!is_same_v<remove_cv_t<T>, in_place_t>, "T must not be in_place_t");
    static_assert(!is_same_v<remove_cv_t<T>, unexpect_t>, "T must not be unexpect_t");
    static_assert(!detail::is_unexpected_v<remove_cv_t<T>>, "T must not be an unexpected type");
    static_assert(
        is_object_v<E> && !is_array_v<E> && !is_const_v<E> && !is_volatile_v<E>,
        "std::expected error type E must be a non-const, non-volatile object type that is not an "
        "array."
    );

    template <typename U>
    static constexpr bool is_other_expected = false;
    template <typename U, typename G>
    static constexpr bool is_other_expected<expected<U, G>> = true;

    public:
    //------------------------------------------------------------------------------//
    // Member Types
    //------------------------------------------------------------------------------//
    using value_type      = T;
    using error_type      = E;
    using unexpected_type = unexpected<E>;

    template <class U>
    using rebind = expected<U, error_type>;

    //------------------------------------------------------------------------------//
    // Construction
    //------------------------------------------------------------------------------//

    constexpr expected()
        requires(is_default_constructible_v<T>)
    noexcept(is_nothrow_default_constructible_v<T>) : value_(), has_value_(true)
    {
    }

    constexpr expected(const expected& rhs)
        requires(is_copy_constructible_v<T> && is_copy_constructible_v<E>)
    noexcept(is_nothrow_copy_constructible_v<T> && is_nothrow_copy_constructible_v<E>)
    {
        has_value_ = rhs.has_value_;
        if (has_value_) {
            std::construct_at(&value_, rhs.value_);
        } else {
            std::construct_at(&error_, rhs.error_);
        }
    }

    constexpr expected(const expected&)
        requires is_trivially_copy_constructible_v<T> && is_trivially_copy_constructible_v<E>
    = default;

    constexpr expected(
        expected&& rhs
    ) noexcept(is_nothrow_move_constructible_v<T> && is_nothrow_move_constructible_v<E>)
        requires is_move_constructible_v<T> && is_move_constructible_v<E>
    {
        has_value_ = rhs.has_value_;
        if (has_value_) {
            std::construct_at(&value_, std::move(rhs.value_));
        } else {
            std::construct_at(&error_, std::move(rhs.error_));
        }
    }

    constexpr expected(expected&&)
        requires is_trivially_move_constructible_v<T> && is_trivially_move_constructible_v<E>
    = default;

    template <typename U, typename G>
        requires is_constructible_v<T, const U&> && is_constructible_v<E, const G&> &&
                 (!is_convertible_v<const expected<U, G>&, T>)
    constexpr explicit(!is_convertible_v<const U&, T> || !is_convertible_v<const G&, E>) expected(
        const expected<U, G>& rhs
    )
        : has_value_(rhs.has_value())
    {
        if (rhs.has_value()) {
            std::construct_at(&value_, *rhs);
        } else {
            std::construct_at(&error_, rhs.error());
        }
    }

    template <typename U, typename G>
        requires is_constructible_v<T, U&&> && is_constructible_v<E, G&&> &&
                 (!is_convertible_v<expected<U, G> &&, T>)
    constexpr explicit(!is_convertible_v<U&&, T> || !is_convertible_v<G&&, E>) expected(
        expected<U, G>&& rhs
    )
        : has_value_(rhs.has_value())
    {
        if (rhs.has_value()) {
            std::construct_at(&value_, std::move(*rhs));
        } else {
            std::construct_at(&error_, std::move(rhs.error()));
        }
    }

    template <typename U = T>
        requires(!std::is_same_v<std::remove_cvref_t<U>, expected>) &&
                    (!detail::is_unexpected_v<std::remove_cvref_t<U>>) &&
                    (!std::is_same_v<std::remove_cvref_t<U>, in_place_t>) &&
                    is_constructible_v<T, U>
    constexpr explicit(!is_convertible_v<U, T>) expected(
        U&& v
    ) noexcept(is_nothrow_constructible_v<T, U>)
        : value_(std::forward<U>(v)), has_value_(true)
    {
    }

    template <typename G>
        requires is_constructible_v<E, const G&>
    constexpr explicit(!is_convertible_v<const G&, E>) expected(
        const unexpected<G>& e
    ) noexcept(is_nothrow_constructible_v<E, const G&>)
        : error_(e.error()), has_value_(false)
    {
    }

    template <typename G>
        requires is_constructible_v<E, G&&>
    constexpr explicit(!is_convertible_v<G&&, E>) expected(
        unexpected<G>&& e
    ) noexcept(is_nothrow_constructible_v<E, G&&>)
        : error_(std::move(e.error())), has_value_(false)
    {
    }

    template <typename... Args>
        requires is_constructible_v<T, Args...>
    constexpr explicit expected(
        in_place_t, Args&&... args
    ) noexcept(is_nothrow_constructible_v<T, Args...>)
        : value_(std::forward<Args>(args)...), has_value_(true)
    {
    }

    template <typename U, typename... Args>
        requires is_constructible_v<T, initializer_list<U>&, Args...>
    constexpr explicit expected(
        in_place_t, initializer_list<U> il, Args&&... args
    ) noexcept(is_nothrow_constructible_v<T, initializer_list<U>&, Args...>)
        : value_(il, std::forward<Args>(args)...), has_value_(true)
    {
    }

    template <typename... Args>
        requires is_constructible_v<E, Args...>
    constexpr explicit expected(
        unexpect_t, Args&&... args
    ) noexcept(is_nothrow_constructible_v<E, Args...>)
        : error_(std::forward<Args>(args)...), has_value_(false)
    {
    }

    template <typename U, typename... Args>
        requires is_constructible_v<E, initializer_list<U>&, Args...>
    constexpr explicit expected(
        unexpect_t, initializer_list<U> il, Args&&... args
    ) noexcept(is_nothrow_constructible_v<E, initializer_list<U>&, Args...>)
        : error_(il, std::forward<Args>(args)...), has_value_(false)
    {
    }

    // Destructor
    constexpr ~expected()
    {
        if (has_value_) {
            std::destroy_at(&value_);
        } else {
            std::destroy_at(&error_);
        }
    }
    constexpr ~expected()
        requires is_trivially_destructible_v<T> && is_trivially_destructible_v<E>
    = default;

    // Assignment
    constexpr expected& operator=(const expected& rhs)
        requires is_copy_assignable_v<T> && is_copy_constructible_v<T> && is_copy_assignable_v<E> &&
                 is_copy_constructible_v<E>
    {
        if (has_value() && rhs.has_value()) {
            value_ = rhs.value_;
        } else if (has_value()) {
            std::destroy_at(&value_);
            std::construct_at(&error_, rhs.error_);
            has_value_ = false;
        } else if (rhs.has_value()) {
            std::destroy_at(&error_);
            std::construct_at(&value_, rhs.value_);
            has_value_ = true;
        } else {
            error_ = rhs.error_;
        }
        return *this;
    }

    constexpr expected& operator=(expected&& rhs) noexcept(
        is_nothrow_move_assignable_v<T> && is_nothrow_move_constructible_v<T> &&
        is_nothrow_move_assignable_v<E> && is_nothrow_move_constructible_v<E>
    )
        requires is_move_assignable_v<T> && is_move_constructible_v<T> && is_move_assignable_v<E> &&
                 is_move_constructible_v<E>
    {
        if (has_value() && rhs.has_value()) {
            value_ = std::move(rhs.value_);
        } else if (has_value()) {
            std::destroy_at(&value_);
            std::construct_at(&error_, std::move(rhs.error_));
            has_value_ = false;
        } else if (rhs.has_value()) {
            std::destroy_at(&error_);
            std::construct_at(&value_, std::move(rhs.value_));
            has_value_ = true;
        } else {
            error_ = std::move(rhs.error_);
        }
        return *this;
    }

    template <typename U = T>
        requires(!is_other_expected<remove_cvref_t<U>>) && is_constructible_v<T, U> &&
                is_assignable_v<T&, U>
    constexpr expected& operator=(U&& v)
    {
        if (has_value()) {
            value_ = std::forward<U>(v);
        } else {
            std::destroy_at(&error_);
            std::construct_at(&value_, std::forward<U>(v));
            has_value_ = true;
        }
        return *this;
    }

    template <typename G>
        requires is_constructible_v<E, const G&> && is_assignable_v<E&, const G&>
    constexpr expected& operator=(const unexpected<G>& e)
    {
        if (has_value()) {
            std::destroy_at(&value_);
            std::construct_at(&error_, e.error());
            has_value_ = false;
        } else {
            error_ = e.error();
        }
        return *this;
    }

    template <typename G>
        requires is_constructible_v<E, G&&> && is_assignable_v<E&, G&&>
    constexpr expected& operator=(unexpected<G>&& e)
    {
        if (has_value()) {
            std::destroy_at(&value_);
            std::construct_at(&error_, std::move(e.error()));
            has_value_ = false;
        } else {
            error_ = std::move(e.error());
        }
        return *this;
    }

    // Modifiers
    template <typename... Args>
        requires is_nothrow_constructible_v<T, Args...>
    constexpr T& emplace(Args&&... args) noexcept
    {
        if (has_value()) {
            std::destroy_at(&value_);
        } else {
            std::destroy_at(&error_);
            has_value_ = true;
        }
        return *std::construct_at(&value_, std::forward<Args>(args)...);
    }

    template <typename U, typename... Args>
        requires is_nothrow_constructible_v<T, initializer_list<U>&, Args...>
    constexpr T& emplace(initializer_list<U> il, Args&&... args) noexcept
    {
        if (has_value()) {
            std::destroy_at(&value_);
        } else {
            std::destroy_at(&error_);
            has_value_ = true;
        }
        return *std::construct_at(&value_, il, std::forward<Args>(args)...);
    }

    // Swap
    constexpr void swap(expected& rhs) noexcept(
        is_nothrow_move_constructible_v<T> && is_nothrow_swappable_v<T> &&
        is_nothrow_move_constructible_v<E> && is_nothrow_swappable_v<E>
    )
        requires is_swappable_v<T> && is_swappable_v<E> && is_move_constructible_v<T> &&
                 is_move_constructible_v<E>
    {
        if (has_value() && rhs.has_value()) {
            using std::swap;
            swap(value_, rhs.value_);
        } else if (!has_value() && !rhs.has_value()) {
            using std::swap;
            swap(error_, rhs.error_);
        } else if (has_value() && !rhs.has_value()) {
            E temp = std::move(rhs.error_);
            std::destroy_at(&rhs.error_);
            std::construct_at(&rhs.value_, std::move(value_));
            std::destroy_at(&value_);
            std::construct_at(&error_, std::move(temp));
            std::swap(has_value_, rhs.has_value_);
        } else {  // !has_value() && rhs.has_value()
            rhs.swap(*this);
        }
    }

    // Observers
    NODISCARD constexpr const T* operator->() const noexcept
    {
        R_ASSERT(has_value_);
        return &value_;
    }
    NODISCARD constexpr T* operator->() noexcept
    {
        R_ASSERT(has_value_);
        return &value_;
    }

    NODISCARD constexpr const T& operator*() const& noexcept
    {
        R_ASSERT(has_value_);
        return value_;
    }
    NODISCARD constexpr T& operator*() & noexcept
    {
        R_ASSERT(has_value_);
        return value_;
    }
    NODISCARD constexpr const T&& operator*() const&& noexcept
    {
        R_ASSERT(has_value_);
        return std::move(value_);
    }
    NODISCARD constexpr T&& operator*() && noexcept
    {
        R_ASSERT(has_value_);
        return std::move(value_);
    }

    NODISCARD constexpr explicit operator bool() const noexcept { return has_value_; }
    NODISCARD constexpr bool has_value() const noexcept { return has_value_; }

    constexpr T& value() &
    {
        if (!has_value()) {
            BAD_EXPECTED_ACCESS("bad access to std::expected without expected value");
        }
        return value_;
    }
    constexpr const T& value() const&
    {
        if (!has_value()) {
            BAD_EXPECTED_ACCESS("bad access to std::expected without expected value");
        }
        return value_;
    }
    constexpr T&& value() &&
    {
        if (!has_value()) {
            BAD_EXPECTED_ACCESS("bad access to std::expected without expected value");
        }
        return std::move(value_);
    }
    constexpr const T&& value() const&&
    {
        if (!has_value()) {
            BAD_EXPECTED_ACCESS("bad access to std::expected without expected value");
        }
        return std::move(value_);
    }

    constexpr const E& error() const& noexcept
    {
        R_ASSERT(!has_value_);
        return error_;
    }
    constexpr E& error() & noexcept
    {
        R_ASSERT(!has_value_);
        return error_;
    }
    constexpr const E&& error() const&& noexcept
    {
        R_ASSERT(!has_value_);
        return std::move(error_);
    }
    constexpr E&& error() && noexcept
    {
        R_ASSERT(!has_value_);
        return std::move(error_);
    }

    template <typename U>
    constexpr T value_or(U&& default_value) const&
    {
        return has_value() ? **this : static_cast<T>(std::forward<U>(default_value));
    }
    template <typename U>
    constexpr T value_or(U&& default_value) &&
    {
        return has_value() ? std::move(**this) : static_cast<T>(std::forward<U>(default_value));
    }

    template <typename G>
    constexpr E error_or(G&& default_error) const&
        requires is_copy_constructible_v<E> && is_convertible_v<G, E>
    {
        return has_value() ? static_cast<E>(std::forward<G>(default_error)) : error_;
    }

    template <typename G>
    constexpr E error_or(G&& default_error) &&
        requires is_move_constructible_v<E> && is_convertible_v<G, E>
    {
        return has_value() ? static_cast<E>(std::forward<G>(default_error)) : std::move(error_);
    }

    // Monadic operations
    template <typename F>
    constexpr auto and_then(F&& f) &
    {
        using result_t = invoke_result_t<F, T&>;
        if (has_value())
            return std::invoke(std::forward<F>(f), **this);
        return result_t(unexpect, error());
    }
    template <typename F>
    constexpr auto and_then(F&& f) const&
    {
        using result_t = invoke_result_t<F, const T&>;
        if (has_value())
            return std::invoke(std::forward<F>(f), **this);
        return result_t(unexpect, error());
    }
    template <typename F>
    constexpr auto and_then(F&& f) &&
    {
        using result_t = invoke_result_t<F, T&&>;
        if (has_value())
            return std::invoke(std::forward<F>(f), std::move(**this));
        return result_t(unexpect, std::move(error()));
    }
    template <typename F>
    constexpr auto and_then(F&& f) const&&
    {
        using result_t = invoke_result_t<F, const T&&>;
        if (has_value())
            return std::invoke(std::forward<F>(f), std::move(**this));
        return result_t(unexpect, std::move(error()));
    }

    template <typename F>
    constexpr auto or_else(F&& f) &
    {
        using result_t = invoke_result_t<F, E&>;
        if (has_value())
            return result_t(in_place, **this);
        return std::invoke(std::forward<F>(f), error());
    }
    template <typename F>
    constexpr auto or_else(F&& f) const&
    {
        using result_t = invoke_result_t<F, const E&>;
        if (has_value())
            return result_t(in_place, **this);
        return std::invoke(std::forward<F>(f), error());
    }
    template <typename F>
    constexpr auto or_else(F&& f) &&
    {
        using result_t = invoke_result_t<F, E&&>;
        if (has_value())
            return result_t(in_place, std::move(**this));
        return std::invoke(std::forward<F>(f), std::move(error()));
    }
    template <typename F>
    constexpr auto or_else(F&& f) const&&
    {
        using result_t = invoke_result_t<F, const E&&>;
        if (has_value())
            return result_t(in_place, std::move(**this));
        return std::invoke(std::forward<F>(f), std::move(error()));
    }

    template <typename F>
    constexpr auto transform(F&& f) &
    {
        using U = remove_cv_t<invoke_result_t<F, T&>>;
        if (has_value())
            return expected<U, E>(std::invoke(std::forward<F>(f), **this));
        return expected<U, E>(unexpect, error());
    }
    template <typename F>
    constexpr auto transform(F&& f) const&
    {
        using U = remove_cv_t<invoke_result_t<F, const T&>>;
        if (has_value())
            return expected<U, E>(std::invoke(std::forward<F>(f), **this));
        return expected<U, E>(unexpect, error());
    }
    template <typename F>
    constexpr auto transform(F&& f) &&
    {
        using U = remove_cv_t<invoke_result_t<F, T&&>>;
        if (has_value())
            return expected<U, E>(std::invoke(std::forward<F>(f), std::move(**this)));
        return expected<U, E>(unexpect, std::move(error()));
    }
    template <typename F>
    constexpr auto transform(F&& f) const&&
    {
        using U = remove_cv_t<invoke_result_t<F, const T&&>>;
        if (has_value())
            return expected<U, E>(std::invoke(std::forward<F>(f), std::move(**this)));
        return expected<U, E>(unexpect, std::move(error()));
    }

    template <typename F>
    constexpr auto transform_error(F&& f) &
    {
        using G = remove_cv_t<invoke_result_t<F, E&>>;
        if (has_value())
            return expected<T, G>(in_place, **this);
        return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), error()));
    }
    template <typename F>
    constexpr auto transform_error(F&& f) const&
    {
        using G = remove_cv_t<invoke_result_t<F, const E&>>;
        if (has_value())
            return expected<T, G>(in_place, **this);
        return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), error()));
    }
    template <typename F>
    constexpr auto transform_error(F&& f) &&
    {
        using G = remove_cv_t<invoke_result_t<F, E&&>>;
        if (has_value())
            return expected<T, G>(in_place, std::move(**this));
        return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), std::move(error())));
    }
    template <typename F>
    constexpr auto transform_error(F&& f) const&&
    {
        using G = remove_cv_t<invoke_result_t<F, const E&&>>;
        if (has_value())
            return expected<T, G>(in_place, std::move(**this));
        return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), std::move(error())));
    }

    // Comparison operators
    template <typename T2, typename E2>
    friend constexpr bool operator==(const expected<T, E>& x, const expected<T2, E2>& y)
    {
        if (x.has_value() != y.has_value())
            return false;
        return x.has_value() ? (*x == *y) : (x.error() == y.error());
    }

    template <typename T2>
    friend constexpr bool operator==(const expected<T, E>& x, const T2& v)
    {
        return x.has_value() && (*x == v);
    }

    template <typename E2>
    friend constexpr bool operator==(const expected<T, E>& x, const unexpected<E2>& e)
    {
        return !x.has_value() && (x.error() == e.error());
    }

    friend void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y)))
        requires requires { x.swap(y); }
    {
        x.swap(y);
    }

    private:
    union {
        T value_;
        E error_;
    };
    bool has_value_;
};

//------------------------------------------------------------------------------//
// Class : Expected<void, E>
//------------------------------------------------------------------------------//
template <class E>
class expected<void, E>
{
    static_assert(
        is_object_v<E> && !is_array_v<E> && !is_const_v<E> && !is_volatile_v<E>,
        "std::expected error type E must be a non-const, non-volatile object type that is not an "
        "array."
    );

    public:
    using value_type      = void;
    using error_type      = E;
    using unexpected_type = unexpected<E>;

    template <class U>
    using rebind = expected<U, error_type>;

    // Construction
    constexpr expected() noexcept : has_value_(true) {}
    constexpr expected(const expected& rhs)
        requires is_copy_constructible_v<E>
    {
        has_value_ = rhs.has_value_;
        if (!has_value_) {
            std::construct_at(&error_, rhs.error_);
        }
    }
    constexpr expected(const expected&)
        requires is_trivially_copy_constructible_v<E>
    = default;

    constexpr expected(expected&& rhs) noexcept(is_nothrow_move_constructible_v<E>)
        requires is_move_constructible_v<E>
    {
        has_value_ = rhs.has_value_;
        if (!has_value_) {
            std::construct_at(&error_, std::move(rhs.error_));
        }
    }
    constexpr expected(expected&&)
        requires is_trivially_move_constructible_v<E>
    = default;

    template <class G>
        requires is_constructible_v<E, const G&>
    constexpr explicit(!is_convertible_v<const G&, E>) expected(
        const expected<void, G>& rhs
    ) noexcept(is_nothrow_constructible_v<E, const G&>)
        : has_value_(rhs.has_value())
    {
        if (!has_value_) {
            std::construct_at(&error_, rhs.error());
        }
    }

    template <class G>
        requires is_constructible_v<E, G>
    constexpr explicit(!is_convertible_v<G, E>) expected(
        expected<void, G>&& rhs
    ) noexcept(is_nothrow_constructible_v<E, G>)
        : has_value_(rhs.has_value())
    {
        if (!has_value_) {
            std::construct_at(&error_, std::move(rhs.error()));
        }
    }

    template <class G>
        requires is_constructible_v<E, const G&>
    constexpr explicit(!is_convertible_v<const G&, E>) expected(
        const unexpected<G>& e
    ) noexcept(is_nothrow_constructible_v<E, const G&>)
        : error_(e.error()), has_value_(false)
    {
    }

    template <class G>
        requires is_constructible_v<E, G>
    constexpr explicit(!is_convertible_v<G, E>) expected(
        unexpected<G>&& e
    ) noexcept(is_nothrow_constructible_v<E, G>)
        : error_(std::move(e.error())), has_value_(false)
    {
    }

    constexpr explicit expected(in_place_t) noexcept : has_value_(true) {}

    template <class... Args>
        requires is_constructible_v<E, Args...>
    constexpr explicit expected(
        unexpect_t, Args&&... args
    ) noexcept(is_nothrow_constructible_v<E, Args...>)
        : error_(std::forward<Args>(args)...), has_value_(false)
    {
    }

    template <class U, class... Args>
        requires is_constructible_v<E, initializer_list<U>&, Args...>
    constexpr explicit expected(
        unexpect_t, initializer_list<U> il, Args&&... args
    ) noexcept(is_nothrow_constructible_v<E, initializer_list<U>&, Args...>)
        : error_(il, std::forward<Args>(args)...), has_value_(false)
    {
    }

    // Destructor
    constexpr ~expected()
    {
        if (!has_value_) {
            std::destroy_at(&error_);
        }
    }
    constexpr ~expected()
        requires is_trivially_destructible_v<E>
    = default;

    // Assignment
    constexpr expected& operator=(const expected& rhs)
        requires is_copy_assignable_v<E> && is_copy_constructible_v<E>
    {
        if (has_value() && !rhs.has_value()) {
            std::construct_at(&error_, rhs.error_);
            has_value_ = false;
        } else if (!has_value() && rhs.has_value()) {
            std::destroy_at(&error_);
            has_value_ = true;
        } else if (!has_value() && !rhs.has_value()) {
            error_ = rhs.error_;
        }
        return *this;
    }

    constexpr expected& operator=(
        expected&& rhs
    ) noexcept(is_nothrow_move_constructible_v<E> && is_nothrow_move_assignable_v<E>)
        requires is_move_constructible_v<E> && is_move_assignable_v<E>
    {
        if (has_value() && !rhs.has_value()) {
            std::construct_at(&error_, std::move(rhs.error_));
            has_value_ = false;
        } else if (!has_value() && rhs.has_value()) {
            std::destroy_at(&error_);
            has_value_ = true;
        } else if (!has_value() && !rhs.has_value()) {
            error_ = std::move(rhs.error_);
        }
        return *this;
    }

    template <class G>
        requires is_constructible_v<E, const G&> && is_assignable_v<E&, const G&>
    constexpr expected& operator=(const unexpected<G>& e)
    {
        if (has_value()) {
            std::construct_at(&error_, e.error());
            has_value_ = false;
        } else {
            error_ = e.error();
        }
        return *this;
    }

    template <class G>
        requires is_constructible_v<E, G> && is_assignable_v<E&, G>
    constexpr expected& operator=(unexpected<G>&& e)
    {
        if (has_value()) {
            std::construct_at(&error_, std::move(e.error()));
            has_value_ = false;
        } else {
            error_ = std::move(e.error());
        }
        return *this;
    }

    // Modifiers
    constexpr void emplace() noexcept
    {
        if (!has_value()) {
            std::destroy_at(&error_);
            has_value_ = true;
        }
    }

    constexpr void swap(
        expected& rhs
    ) noexcept(is_nothrow_move_constructible_v<E> && is_nothrow_swappable_v<E>)
        requires is_swappable_v<E> && is_move_constructible_v<E>
    {
        if (has_value() && rhs.has_value()) {
            // do nothing
        } else if (!has_value() && !rhs.has_value()) {
            using std::swap;
            swap(error_, rhs.error_);
        } else if (has_value() && !rhs.has_value()) {
            std::construct_at(&error_, std::move(rhs.error_));
            std::destroy_at(&rhs.error_);
            has_value_     = false;
            rhs.has_value_ = true;
        } else {
            rhs.swap(*this);
        }
    }

    // Observers
    NODISCARD constexpr const void* operator->() const noexcept
    {
        R_ASSERT(has_value());
        return nullptr;
    }
    NODISCARD constexpr void* operator->() noexcept
    {
        R_ASSERT(has_value());
        return nullptr;
    }

    constexpr void operator*() const noexcept { R_ASSERT(has_value()); }

    NODISCARD constexpr explicit operator bool() const noexcept { return has_value_; }
    NODISCARD constexpr bool has_value() const noexcept { return has_value_; }

    constexpr void value() const&
    {
        if (!has_value()) {
            BAD_EXPECTED_ACCESS("bad access to std::expected without expected value");
        }
    }

    constexpr void value() &&
    {
        if (!has_value()) {
            BAD_EXPECTED_ACCESS("bad access to std::expected without expected value");
        }
    }

    constexpr const E& error() const& noexcept
    {
        R_ASSERT(!has_value_);
        return error_;
    }
    constexpr E& error() & noexcept
    {
        R_ASSERT(!has_value_);
        return error_;
    }
    constexpr const E&& error() const&& noexcept
    {
        R_ASSERT(!has_value_);
        return std::move(error_);
    }
    constexpr E&& error() && noexcept
    {
        R_ASSERT(!has_value_);
        return std::move(error_);
    }

    template <typename G>
    constexpr E error_or(G&& default_error) const&
        requires is_copy_constructible_v<E> && is_convertible_v<G, E>
    {
        return has_value() ? static_cast<E>(std::forward<G>(default_error)) : error_;
    }

    template <typename G>
    constexpr E error_or(G&& default_error) &&
        requires is_move_constructible_v<E> && is_convertible_v<G, E>
    {
        return has_value() ? static_cast<E>(std::forward<G>(default_error)) : std::move(error_);
    }

    // Monadic Operations
    template <class F>
    constexpr auto and_then(F&& f) &
    {
        using result_t = invoke_result_t<F>;
        if (has_value())
            return std::invoke(std::forward<F>(f));
        return result_t(unexpect, error_);
    }
    template <class F>
    constexpr auto and_then(F&& f) const&
    {
        using result_t = invoke_result_t<F>;
        if (has_value())
            return std::invoke(std::forward<F>(f));
        return result_t(unexpect, error_);
    }
    template <class F>
    constexpr auto and_then(F&& f) &&
    {
        using result_t = invoke_result_t<F>;
        if (has_value())
            return std::invoke(std::forward<F>(f));
        return result_t(unexpect, std::move(error_));
    }
    template <class F>
    constexpr auto and_then(F&& f) const&&
    {
        using result_t = invoke_result_t<F>;
        if (has_value())
            return std::invoke(std::forward<F>(f));
        return result_t(unexpect, std::move(error_));
    }

    template <class F>
    constexpr auto or_else(F&& f) &
    {
        using result_t = invoke_result_t<F, E&>;
        if (has_value())
            return result_t(in_place);
        return std::invoke(std::forward<F>(f), error_);
    }
    template <class F>
    constexpr auto or_else(F&& f) const&
    {
        using result_t = invoke_result_t<F, const E&>;
        if (has_value())
            return result_t(in_place);
        return std::invoke(std::forward<F>(f), error_);
    }
    template <class F>
    constexpr auto or_else(F&& f) &&
    {
        using result_t = invoke_result_t<F, E&&>;
        if (has_value())
            return result_t(in_place);
        return std::invoke(std::forward<F>(f), std::move(error_));
    }
    template <class F>
    constexpr auto or_else(F&& f) const&&
    {
        using result_t = invoke_result_t<F, const E&&>;
        if (has_value())
            return result_t(in_place);
        return std::invoke(std::forward<F>(f), std::move(error_));
    }

    template <class F>
    constexpr auto transform(F&& f) &
    {
        using U = remove_cv_t<invoke_result_t<F>>;
        if (has_value()) {
            std::invoke(std::forward<F>(f));
            return expected<U, E>(in_place);
        }
        return expected<U, E>(unexpect, error_);
    }
    template <class F>
    constexpr auto transform(F&& f) const&
    {
        using U = remove_cv_t<invoke_result_t<F>>;
        if (has_value()) {
            std::invoke(std::forward<F>(f));
            return expected<U, E>(in_place);
        }
        return expected<U, E>(unexpect, error_);
    }
    template <class F>
    constexpr auto transform(F&& f) &&
    {
        using U = remove_cv_t<invoke_result_t<F>>;
        if (has_value()) {
            std::invoke(std::forward<F>(f));
            return expected<U, E>(in_place);
        }
        return expected<U, E>(unexpect, std::move(error_));
    }
    template <class F>
    constexpr auto transform(F&& f) const&&
    {
        using U = remove_cv_t<invoke_result_t<F>>;
        if (has_value()) {
            std::invoke(std::forward<F>(f));
            return expected<U, E>(in_place);
        }
        return expected<U, E>(unexpect, std::move(error_));
    }

    template <class F>
    constexpr auto transform_error(F&& f) &
    {
        using G = remove_cv_t<invoke_result_t<F, E&>>;
        if (has_value())
            return expected<void, G>(in_place);
        return expected<void, G>(unexpect, std::invoke(std::forward<F>(f), error_));
    }
    template <class F>
    constexpr auto transform_error(F&& f) const&
    {
        using G = remove_cv_t<invoke_result_t<F, const E&>>;
        if (has_value())
            return expected<void, G>(in_place);
        return expected<void, G>(unexpect, std::invoke(std::forward<F>(f), error_));
    }
    template <class F>
    constexpr auto transform_error(F&& f) &&
    {
        using G = remove_cv_t<invoke_result_t<F, E&&>>;
        if (has_value())
            return expected<void, G>(in_place);
        return expected<void, G>(unexpect, std::invoke(std::forward<F>(f), std::move(error_)));
    }
    template <class F>
    constexpr auto transform_error(F&& f) const&&
    {
        using G = remove_cv_t<invoke_result_t<F, const E&&>>;
        if (has_value())
            return expected<void, G>(in_place);
        return expected<void, G>(unexpect, std::invoke(std::forward<F>(f), std::move(error_)));
    }

    // Comparison operators
    template <class E2>
    friend constexpr bool operator==(const expected<void, E>& x, const expected<void, E2>& y)
    {
        if (x.has_value() != y.has_value())
            return false;
        return x.has_value() ? true : (x.error() == y.error());
    }

    template <class E2>
    friend constexpr bool operator==(const expected<void, E>& x, const unexpected<E2>& e)
    {
        return !x.has_value() && (x.error() == e.error());
    }

    friend void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y)))
        requires requires { x.swap(y); }
    {
        x.swap(y);
    }

    private:
    union {
        // No value member for void specialization
        E error_;
    };
    bool has_value_;
};

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_EXPECTED_HPP_
