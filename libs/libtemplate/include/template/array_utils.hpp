#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_ARRAY_UTILS_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_ARRAY_UTILS_HPP_

#include <cstddef.hpp>

namespace template_lib
{
// ------------------------------
// ArrayHolder
// ------------------------------

template <typename T, T... args>
struct ArrayHolder {
    static constexpr T data[sizeof...(args)] = {args...};
};

template <typename T, T... args>
constexpr T ArrayHolder<T, args...>::data[sizeof...(args)];

template <typename T, typename Pack1, typename Pack2>
struct ConcatenateArrays;

template <typename T, T... pack1_args, T... pack2_args>
struct ConcatenateArrays<T, ArrayHolder<T, pack1_args...>, ArrayHolder<T, pack2_args...>> {
    using type = ArrayHolder<T, pack1_args..., pack2_args...>;
};

// ------------------------------
// MakeUniformArray
// ------------------------------

template <typename T, T value, size_t N, T... args>
struct MakeUniformArray {
    using type = typename MakeUniformArray<T, value, N - 1, args..., value>::type;
};

template <typename T, T value, T... args>
struct MakeUniformArray<T, value, 0, args...> {
    using type = ArrayHolder<T, args...>;
};

template <typename T, T fill, size_t N>
using uniform_array_t = typename MakeUniformArray<T, fill, N>::type;

template <typename T, T fill, size_t N>
constexpr auto uniform_array_v = uniform_array_t<T, fill, N>::data;

// ------------------------------
// MakeFilledArray
// ------------------------------

template <typename T, size_t N, T fill, T... args>
struct MakeFilledArray {
    static_assert(sizeof...(args) <= N, "Too many initial arguments for array size");

    private:
    using initial_array = ArrayHolder<T, args...>;
    using uniform_array = typename MakeUniformArray<T, fill, N - sizeof...(args)>::type;

    public:
    using type = typename ConcatenateArrays<T, initial_array, uniform_array>::type;
};

template <typename T, size_t N, T fill, T... args>
using fill_array_t = typename MakeFilledArray<T, N, fill, args...>::type;

template <typename T, size_t N, T fill, T... args>
constexpr auto fill_array_v = fill_array_t<T, N, fill, args...>::data;

}  // namespace template_lib

#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_ARRAY_UTILS_HPP_
