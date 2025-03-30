#ifndef LIBC_INCLUDE_EXTENSIONS_ALGORITHM_HPP_
#define LIBC_INCLUDE_EXTENSIONS_ALGORITHM_HPP_

#include "todo.h"

namespace std
{

// ------------------------------
// std::max
// ------------------------------

template <class T>
constexpr const T& max(const T& a, const T& b)
{
    return (a < b) ? b : a;
}

template <class T, class Compare>
constexpr const T& max(const T& a, const T& b, Compare comp)
{
    return (comp(a, b)) ? b : a;
}

// ------------------------------
// std::min
// ------------------------------

template <class T>
constexpr const T& min(const T& a, const T& b)
{
    return (b < a) ? b : a;
}

template <class T, class Compare>
constexpr const T& min(const T& a, const T& b, Compare comp)
{
    return (comp(b, a)) ? b : a;
}

template <class InputIt, class OutputIt>
constexpr OutputIt copy(InputIt first, InputIt last, OutputIt d_first)
{
    // Read https://en.cppreference.com/w/cpp/algorithm/copy and gcc's implementation
    // Notes for optimization guidelines
    TODO_OPTIMISE
    while (first != last) {
        *d_first++ = *first++;
    }
    return d_first;
}

template <class InputIt, class OutputIt, class UnaryPred>
OutputIt copy_if(InputIt first, InputIt last, OutputIt d_first, UnaryPred pred)
{
    // Read https://en.cppreference.com/w/cpp/algorithm/copy and gcc's implementation
    // Notes for optimization guidelines
    TODO_OPTIMISE
    for (; first != last; ++first)
        if (pred(*first)) {
            *d_first = *first;
            ++d_first;
        }

    return d_first;
}

TODO_LIBCPP_COMPLIANCE
// TODO: Implement std::max with initializer list

};  // namespace std

#endif  // LIBC_INCLUDE_EXTENSIONS_ALGORITHM_HPP_
