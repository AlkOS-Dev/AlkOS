#ifndef LIBC_INCLUDE_EXTENSIONS_ALGORITHM_HPP_
#define LIBC_INCLUDE_EXTENSIONS_ALGORITHM_HPP_

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

// TODO:
// template<class T>
// constexpr T max(std::initializer_list<T> ilist)

// TODO:
// template<class T, class Compare>
// constexpr T max(std::initializer_list<T> ilist, Compare comp)

};  // namespace std

#endif  // LIBC_INCLUDE_EXTENSIONS_ALGORITHM_HPP_
