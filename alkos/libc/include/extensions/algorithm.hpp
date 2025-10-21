#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_ALGORITHM_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_ALGORITHM_HPP_

#include <todo.h>

namespace std
{

// ------------------------------
// std::max
// ------------------------------

template <class T>
constexpr const T &max(const T &a, const T &b)
{
    return (a < b) ? b : a;
}

template <class T, class Compare>
constexpr const T &max(const T &a, const T &b, Compare comp)
{
    return (comp(a, b)) ? b : a;
}

// ------------------------------
// std::min
// ------------------------------

template <class T>
constexpr const T &min(const T &a, const T &b)
{
    return (b < a) ? b : a;
}

template <class T, class Compare>
constexpr const T &min(const T &a, const T &b, Compare comp)
{
    return (comp(b, a)) ? b : a;
}

TODO_LIBCPP_COMPLIANCE
// TODO: Implement std::max with initializer list

};  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_ALGORITHM_HPP_
