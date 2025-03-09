#ifndef LIBC_INCLUDE_TEMPLATE_LIB_HPP_
#define LIBC_INCLUDE_TEMPLATE_LIB_HPP_

#include <extensions/concepts_ext.hpp>
#include <extensions/defines.hpp>
#include <extensions/new.hpp>
#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>
#include <extensions/utility.hpp>

#include <assert.h>

namespace TemplateLib
{
// ------------------------------
// Rolled Switch
// ------------------------------

template <class ExprT, u64 kMaxValue, u64 kStep, class FuncT, class DefaultFuncT, class... Args>
    requires concepts_ext::Callable<DefaultFuncT, Args...> &&
             concepts_ext::RolledSwitchFunctor<FuncT, ExprT, Args...>
FAST_CALL constexpr void RolledSwitch(
    DefaultFuncT &&default_func, FuncT &&func, const u64 value, Args &&...args
) noexcept
{
    if (value == kMaxValue) {
        func.template operator()<static_cast<ExprT>(kMaxValue)>(std::forward<Args>(args)...);
        return;
    }

    if constexpr (kMaxValue >= kStep) {
        RolledSwitch<ExprT, kMaxValue - kStep, kStep>(
            std::forward<DefaultFuncT>(default_func), std::forward<FuncT>(func), value,
            std::forward<Args>(args)...
        );
        return;
    }

    default_func(std::forward<Args>(args)...);
}

template <class ExprT, u64 kMaxValue, u64 kStep, class FuncT, class DefaultFuncT, class... Args>
    requires concepts_ext::Callable<DefaultFuncT, Args...> &&
             concepts_ext::RolledSwitchFunctor<FuncT, ExprT, Args...>
FAST_CALL constexpr auto RolledSwitchReturnable(
    DefaultFuncT &&default_func, FuncT &&func, const u64 value, Args &&...args
) noexcept
{
    if (value == kMaxValue) {
        return func.template operator()<static_cast<ExprT>(kMaxValue)>(std::forward<Args>(args)...);
    }

    if constexpr (kMaxValue >= kStep) {
        return RolledSwitchReturnable<ExprT, kMaxValue - kStep, kStep>(
            std::forward<DefaultFuncT>(default_func), std::forward<FuncT>(func), value,
            std::forward<Args>(args)...
        );
    }

    return default_func(std::forward<Args>(args)...);
}

template <class ExprT, u64 kMaxValue, u64 kStep, class FuncT, class... Args>
    requires concepts_ext::RolledSwitchFunctor<FuncT, ExprT, Args...>
FAST_CALL constexpr void RolledSwitch(FuncT &&func, const u64 value, Args &&...args) noexcept
{
    RolledSwitch<ExprT, kMaxValue, kStep>(
        []() constexpr FORCE_INLINE_L {
            R_FAIL_ALWAYS("Switch out of range error...");
        },
        std::forward<FuncT>(func), value, std::forward<Args>(args)...
    );
}

template <class ExprT, u64 kMaxValue, u64 kStep, class FuncT, class... Args>
    requires concepts_ext::RolledSwitchFunctor<FuncT, ExprT, Args...>
FAST_CALL constexpr auto RolledSwitchReturnable(
    FuncT &&func, const u64 value, Args &&...args
) noexcept
{
    return RolledSwitchReturnable<ExprT, kMaxValue, kStep>(
        []() constexpr FORCE_INLINE_L {
            R_FAIL_ALWAYS("Switch out of range error...");
        },
        std::forward<FuncT>(func), value, std::forward<Args>(args)...
    );
}

// ------------------------------
// Type Checks
// ------------------------------

template <typename T, typename... Args>
constexpr size_t CountType()
{
    return (static_cast<size_t>(std::is_same_v<T, Args>) + ...);
}

template <typename T, typename... Args>
constexpr bool HasType()
{
    return (std::is_same_v<T, Args> || ...);
}

template <typename T, typename... Args>
constexpr bool HasTypeOnce()
{
    return CountType<T, Args...>() == 1;
}

template <typename T, typename... Args>
constexpr bool HasDuplicateType()
{
    return CountType<T, Args...>() > 1;
}

template <typename... Args>
constexpr bool HasDuplicateTypes()
{
    return (HasDuplicateType<Args, Args...>() || ...);
}

// ------------------------------
// Type List
// ------------------------------

template <size_t N, class T, class... Ts>
struct TypeList {
    static constexpr size_t size = sizeof...(Ts) + 1;

    static_assert(N < size, "Index out of range");
    using type = typename TypeList<N - 1, Ts...>::type;
};

template <class T, class... Ts>
struct TypeList<0, T, Ts...> {
    static constexpr size_t size = sizeof...(Ts) + 1;

    using type = T;
};

template <size_t N, template <size_t> class TypeList>
struct IterateTypeList {
    template <class Callable, class... Args>
    FORCE_INLINE_F static constexpr void Apply(Callable &&func, Args &&...args)
    {
        func.template operator()<N, typename TypeList<N>::type>(args...);
        IterateTypeList<N - 1, TypeList>::Apply(
            std::forward<Callable>(func), std::forward<Args>(args)...
        );
    }
};

template <template <size_t> class TypeList>
struct IterateTypeList<0, TypeList> {
    template <class Callable>
    FAST_CALL constexpr void Apply(Callable &&func)
    {
        func.template operator()<0, typename TypeList<0>::type>();
    }
};

template <class... Args>
struct IterateTypes {
    static_assert(sizeof...(Args) > 0, "Type list must not be empty");

    template <size_t N>
    using TypeList = TypeList<N, Args...>;

    template <class Callable, class... CallArgs>
    FAST_CALL constexpr void Apply(Callable &&func, CallArgs &&...args)
    {
        IterateTypeList<sizeof...(Args) - 1, TypeList>::Apply(
            std::forward<Callable>(func), std::forward<CallArgs>(args)...
        );
    }
};

template <class T, class... Args>
NODISCARD FAST_CALL constexpr size_t GetTypeIndexInTypes()
{
    static_assert(HasTypeOnce<T, Args...>(), "Type must occur exactly once in the tuple");
    size_t idx{};

    IterateTypes<Args...>::Apply([&]<size_t Index, class U>() {
        if constexpr (std::is_same_v<T, U>) {
            idx = Index;
        }
    });

    return idx;
}

// ------------------------------
// Static singleton
// ------------------------------

class StaticSingletonHelper
{
    public:
    StaticSingletonHelper(StaticSingletonHelper const &)            = delete;
    StaticSingletonHelper &operator=(StaticSingletonHelper const &) = delete;

    StaticSingletonHelper(StaticSingletonHelper &&)            = delete;
    StaticSingletonHelper &operator=(StaticSingletonHelper &&) = delete;

    protected:
    /* Non instantiable */
    StaticSingletonHelper() = default;
};

template <class T>
concept DerivedFromHelper = std::is_base_of_v<StaticSingletonHelper, T>;

template <class T>
    requires DerivedFromHelper<T>
class StaticSingleton
{
    struct InstanceHelper final : T {
        /* Makes protected constructor accessible */

        template <class... Args>
        explicit InstanceHelper(Args &&...args) noexcept : T(std::forward<Args>(args)...)
        {
        }
    };

    public:
    // ------------------------------------
    // Static Accessors and Utilities
    // ------------------------------------

    FORCE_INLINE_F static T &Get() noexcept
    {
        assert(is_instance_inited_ && "Not inited Singleton instance!");
        return *reinterpret_cast<T *>(instance_memory_);
    }

    FORCE_INLINE_F static void Destroy() noexcept
    {
        Get().~T();
        is_instance_inited_ = false;
    }

    FORCE_INLINE_F static bool IsInited() noexcept { return is_instance_inited_; }

    template <class... Args>
    FORCE_INLINE_F static T &Init(Args &&...args) noexcept
    {
        assert(!IsInited() && "Singleton instance already inited!");
        auto ptr = new (instance_memory_) InstanceHelper(std::forward<Args>(args)...);
        assert(ptr == reinterpret_cast<T *>(instance_memory_));

        is_instance_inited_ = true;
        return Get();
    }

    protected:
    // ------------------------------
    // Static memory
    // ------------------------------

    static bool is_instance_inited_;
    alignas(alignof(T)) static unsigned char instance_memory_[sizeof(T)];
};

template <typename T>
    requires DerivedFromHelper<T>
bool StaticSingleton<T>::is_instance_inited_ = false;

template <typename T>
    requires DerivedFromHelper<T>
unsigned char StaticSingleton<T>::instance_memory_[sizeof(T)]{};

}  // namespace TemplateLib
#endif  // LIBC_INCLUDE_TEMPLATE_LIB_HPP_
