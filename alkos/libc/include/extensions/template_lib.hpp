#ifndef LIBC_INCLUDE_TEMPLATE_LIB_HPP_
#define LIBC_INCLUDE_TEMPLATE_LIB_HPP_

#include <assert.h>
#include <extensions/array.hpp>
#include <extensions/concepts_ext.hpp>
#include <extensions/defines.hpp>
#include <extensions/internal/tuple_base.hpp>
#include <extensions/new.hpp>
#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>
#include <extensions/utility.hpp>

namespace TemplateLib
{
// ------------------------------
// NoCopy
// ------------------------------

struct NoCopy {
    NoCopy()  = default;
    ~NoCopy() = default;

    NoCopy(const NoCopy &) = delete;
    NoCopy(NoCopy &&)      = delete;

    NoCopy &operator=(const NoCopy &) = delete;
    NoCopy &operator=(NoCopy &&)      = delete;
};

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

template <class... Args>
struct TypeList {
    // ------------------------------
    // iterators
    // ------------------------------

    template <size_t N, class T, class... Ts>
    struct TypeListIter {
        static constexpr size_t kSize = sizeof...(Ts) + 1;

        static_assert(N < kSize, "Index out of range");
        using type = typename TypeListIter<N - 1, Ts...>::type;
    };

    template <class T, class... Ts>
    struct TypeListIter<0, T, Ts...> {
        static constexpr size_t kSize = sizeof...(Ts) + 1;

        using type = T;
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

// ------------------------------
// Static singleton
// ------------------------------

class StaticSingletonHelper : public NoCopy
{
    protected:
    /* Non instantiable */
    StaticSingletonHelper()  = default;
    ~StaticSingletonHelper() = default;
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

// ------------------------------
// Settings
// ------------------------------

template <class TypeListT>
    requires IsTypeList_v<TypeListT>
class Settings : public NoCopy
{
    // ------------------------------
    // internals
    // ------------------------------

    public:
    using EvenT  = void (*)();
    using TupleT = TypeListT::Tuple;

    private:
    struct NodeT {
        EvenT event;
        NodeT *next;
    };

    static constexpr size_t kMaxEvents = 256;

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    constexpr Settings(TupleT &&settings) : settings_(std::move(settings)) {}

    // ------------------------------
    // Class methods
    // ------------------------------

    template <class T>
        requires std::is_convertible_v<T, size_t>
    FORCE_INLINE_F constexpr auto Get(const T t_item_idx)
    {
        const size_t idx = static_cast<size_t>(t_item_idx);

        ASSERT_LT(idx, TypeListT::kSize);
        return std::get<idx>(settings_);
    }

    template <class T, class U>
        requires std::is_convertible_v<T, size_t>
    FORCE_INLINE_F constexpr void Set(const T t_item_idx, U &&value)
    {
        const size_t idx = static_cast<size_t>(t_item_idx);

        ASSERT_LT(idx, TypeListT::kSize);
        std::get<idx>(settings_) = std::forward<U>(value);
    }

    template <class T, class U>
        requires std::is_convertible_v<T, size_t>
    FORCE_INLINE_F constexpr void Set(const T t_item_idx, const U &value)
    {
        const size_t idx = static_cast<size_t>(t_item_idx);

        ASSERT_LT(idx, TypeListT::kSize);
        std::get<idx>(settings_) = std::forward<U>(value);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    TupleT settings_;
    NodeT *events_[TypeListT::kSize]{};
    NodeT events_mem_[kMaxEvents];
    size_t events_mem_top_{};
};

}  // namespace TemplateLib
#endif  // LIBC_INCLUDE_TEMPLATE_LIB_HPP_
