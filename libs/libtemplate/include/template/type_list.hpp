#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_TYPE_LIST_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_TYPE_LIST_HPP_

#include <internal/tuple_base.hpp>
#include <type_traits.hpp>
#include <types.hpp>
#include <utility.hpp>

#include "type_checks.hpp"

namespace template_lib
{
// ------------------------------
// Type List
// ------------------------------

template <class... Args>
struct TypeList {
    // ------------------------------
    // iterators
    // ------------------------------

    template <size_t N, class... Ts>
    struct TypeListIter;

    template <size_t N, class T, class... Ts>
    struct TypeListIter<N, T, Ts...> {
        static constexpr size_t kSize = sizeof...(Ts) + 1;

        static_assert(N < kSize, "Index out of range");
        using type = typename TypeListIter<N - 1, Ts...>::type;
    };

    template <class T, class... Ts>
    struct TypeListIter<0, T, Ts...> {
        static constexpr size_t kSize = sizeof...(Ts) + 1;

        using type = T;
    };

    template <size_t N>
    struct TypeListIter<N> {
        static constexpr size_t kSize = 0;
        using type                    = void;
    };

    // ------------------------------
    // Invokers
    // ------------------------------

    template <size_t N, class Callable, class... ApplyArgs>
    FORCE_INLINE_F static constexpr void Apply(Callable &&func, ApplyArgs &&...args)
    {
        func.template operator()<N, typename Iterator<N>::type>(args...);

        if constexpr (N > 0) {
            Apply<N - 1>(std::forward<Callable>(func), std::forward<ApplyArgs>(args)...);
        }
    }

    template <class Callable, class... ApplyArgs>
    FORCE_INLINE_F static constexpr void Apply(Callable &&func, ApplyArgs &&...args)
    {
        Apply<kSize - 1>(std::forward<Callable>(func), std::forward<ApplyArgs>(args)...);
    }

    // ------------------------------
    // Fields
    // ------------------------------

    static constexpr size_t kSize = sizeof...(Args);

    template <size_t N>
    using Iterator = TypeListIter<N, Args...>;
    using Tuple    = std::tuple<Args...>;
};

template <class T, class... Args>
NODISCARD FAST_CALL constexpr size_t GetTypeIndexInTypes()
{
    static_assert(HasTypeOnce<T, Args...>(), "Type must occur exactly once in the tuple");
    size_t idx{};

    TypeList<Args...>::Apply([&]<size_t Index, class U>() {
        if constexpr (std::is_same_v<T, U>) {
            idx = Index;
        }
    });

    return idx;
}

template <class T>
struct IsTypeList : std::false_type {
};

template <class... Args>
struct IsTypeList<TypeList<Args...>> : std::true_type {
};

template <class T>
constexpr bool IsTypeList_v = IsTypeList<T>::value;
}  // namespace template_lib
#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_TYPE_LIST_HPP_
