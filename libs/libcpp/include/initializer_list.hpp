#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INITIALIZER_LIST_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INITIALIZER_LIST_HPP_

#include <cstddef.hpp>
#include <defines.hpp>

namespace std
{

template <class T>
class initializer_list
{
    public:
    // ------------------------------
    // Member types
    // ------------------------------

    using value_type      = T;
    using reference       = T &;
    using const_reference = const T &;
    using size_type       = size_t;
    using iterator        = const T *;
    using const_iterator  = const T *;

    // ------------------------------
    // Class creation
    // ------------------------------

    private:
    /* For compiler */
    constexpr initializer_list(const T *items, size_t len) : items_(items), len_(len) {}

    public:
    constexpr initializer_list() noexcept : items_(nullptr), len_(0_size) {}

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD constexpr size_type size() const noexcept { return len_; }

    NODISCARD constexpr const_iterator begin() const noexcept { return items_; }

    NODISCARD constexpr const_iterator end() const noexcept { return begin() + size(); }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    const T *items_;
    size_t len_;
};

template <class T>
constexpr const T *begin(initializer_list<T> list) noexcept
{
    return list.begin();
}

template <class T>
constexpr const T *end(initializer_list<T> list) noexcept
{
    return list.end();
}

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INITIALIZER_LIST_HPP_
