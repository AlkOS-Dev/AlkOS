#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_TEMPLATE_LIB_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_TEMPLATE_LIB_HPP_

#include <assert.h>
#include <extensions/array.hpp>
#include <extensions/concepts_ext.hpp>
#include <extensions/defines.hpp>
#include <extensions/internal/tuple_base.hpp>
#include <extensions/new.hpp>
#include <extensions/optional.hpp>
#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>
#include <extensions/utility.hpp>

namespace template_lib
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
// MoveOnly
// ------------------------------

struct MoveOnly {
    MoveOnly()  = default;
    ~MoveOnly() = default;

    /* remove copying */
    MoveOnly(const MoveOnly &)            = delete;
    MoveOnly &operator=(const MoveOnly &) = delete;

    /* allow moving */
    MoveOnly(MoveOnly &&)            = default;
    MoveOnly &operator=(MoveOnly &&) = default;
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
    return concepts_ext::OneOf<T, Args...>;
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
class SingletonInstanceCreator
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

    FORCE_INLINE_F T &Get() noexcept
    {
        assert(is_instance_inited_ && "Not inited Singleton instance!");
        return *reinterpret_cast<T *>(instance_memory_);
    }

    FORCE_INLINE_F void Destroy() noexcept
    {
        Get().~T();
        is_instance_inited_ = false;
    }

    FORCE_INLINE_F bool IsInited() noexcept { return is_instance_inited_; }

    template <class... Args>
    FORCE_INLINE_F T &Init(Args &&...args) noexcept
    {
        assert(!IsInited() && "Singleton instance already inited!");
        [[maybe_unused]] auto ptr =
            new (instance_memory_) InstanceHelper(std::forward<Args>(args)...);
        assert(ptr == reinterpret_cast<T *>(instance_memory_));

        is_instance_inited_ = true;
        return Get();
    }

    protected:
    // ------------------------------
    // Static memory
    // ------------------------------

    bool is_instance_inited_ = false;
    alignas(alignof(T)) unsigned char instance_memory_[sizeof(T)]{};
};

template <class T>
    requires DerivedFromHelper<T>
class StaticSingleton
{
    public:
    // ------------------------------------
    // Static Accessors and Utilities
    // ------------------------------------

    FORCE_INLINE_F static T &Get() noexcept { return instance_creator_.Get(); }

    FORCE_INLINE_F static void Destroy() noexcept { instance_creator_.Destroy(); }

    FORCE_INLINE_F static bool IsInited() noexcept { return instance_creator_.IsInited(); }

    template <class... Args>
    FORCE_INLINE_F static T &Init(Args &&...args) noexcept
    {
        return instance_creator_.Init(std::forward<Args>(args)...);
    }

    protected:
    // ------------------------------
    // Static memory
    // ------------------------------

    static SingletonInstanceCreator<T> instance_creator_;
};

template <class T>
    requires DerivedFromHelper<T>
SingletonInstanceCreator<T> StaticSingleton<T>::instance_creator_;

// ------------------------------
// EventTable
// ------------------------------

template <size_t kSize, class EvenT>
class StaticEventTable : MoveOnly
{
    // ------------------------------
    // internals
    // ------------------------------

    struct NodeT {
        EvenT event;
        NodeT *next;
    };

    static constexpr size_t kMaxEvents = 256;

    // ------------------------------
    // Class construction
    // ------------------------------

    public:
    StaticEventTable() noexcept  = default;
    ~StaticEventTable() noexcept = default;

    // ------------------------------
    // Class methods
    // ------------------------------

    template <size_t idx>
    void RegisterEvent(EvenT &&event)
    {
        static_assert(idx < kSize, "Index out of range");

        NodeT *const node = &events_mem_[events_mem_top_++];
        node->event       = std::move(event);
        node->next        = nullptr;

        NodeT **cur_node = &events_[idx];
        while (*cur_node) {
            cur_node = &(*cur_node)->next;
        }
        *cur_node = node;
    }

    template <size_t idx, class... Args>
        requires std::is_invocable_v<EvenT, Args...>
    void Notify(Args &&...args)
    {
        static_assert(idx < kSize, "Index out of range");

        NodeT *cur_node = events_[idx];
        while (cur_node) {
            cur_node->event(args...);
            cur_node = cur_node->next;
        }
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    NodeT *events_[kSize]{};
    NodeT events_mem_[kMaxEvents];
    size_t events_mem_top_{};
};

// ------------------------------
// Settings
// ------------------------------

template <class TypeListT, class AccessT = size_t>
    requires IsTypeList_v<TypeListT> && requires { static_cast<size_t>(std::declval<AccessT>()); }
class Settings : public NoCopy
{
    public:
    using TupleT = typename TypeListT::Tuple;
    using EventT = void (*)();

    // ------------------------------
    // Class creation
    // ------------------------------

    explicit constexpr Settings(TupleT &&settings) : settings_(std::move(settings)) {}
    explicit constexpr Settings(const TupleT &settings) : settings_(settings) {}

    // ------------------------------
    // Class methods
    // ------------------------------

    template <AccessT idx>
    FORCE_INLINE_F constexpr auto Get()
    {
        static_assert(static_cast<size_t>(idx) < TypeListT::kSize, "Index out of range");
        return settings_.template get<static_cast<size_t>(idx)>();
    }

    template <AccessT idx, class U>
    FORCE_INLINE_F constexpr void Set(U &&value)
    {
        using OptType = typename TypeListT::template Iterator<static_cast<size_t>(idx)>::type;
        static_assert(static_cast<size_t>(idx) < TypeListT::kSize, "Index out of range");
        static_assert(std::is_same_v<U, OptType>, "Invalid option type");

        settings_.template get<static_cast<size_t>(idx)>() = std::forward<U>(value);
    }

    template <AccessT idx, class U>
    FORCE_INLINE_F constexpr void Set(const U &value)
    {
        using OptType = typename TypeListT::template Iterator<static_cast<size_t>(idx)>::type;
        static_assert(static_cast<size_t>(idx) < TypeListT::kSize, "Index out of range");
        static_assert(std::is_same_v<U, OptType>, "Invalid option type");

        settings_.template get<static_cast<size_t>(idx)>() = std::forward<U>(value);
    }

    template <AccessT idx, class U>
    FORCE_INLINE_F constexpr void SetAndNotify(U &&value)
    {
        using OptType = typename TypeListT::template Iterator<static_cast<size_t>(idx)>::type;
        static_assert(static_cast<size_t>(idx) < TypeListT::kSize, "Index out of range");
        static_assert(std::is_same_v<U, OptType>, "Invalid option type");

        settings_.template get<static_cast<size_t>(idx)>() = std::forward<U>(value);
        event_table_.template Notify<static_cast<size_t>(idx)>();
    }

    template <AccessT idx, class U>
    FORCE_INLINE_F constexpr void SetAndNotify(const U &value)
    {
        using OptType = typename TypeListT::template Iterator<static_cast<size_t>(idx)>::type;
        static_assert(static_cast<size_t>(idx) < TypeListT::kSize, "Index out of range");
        static_assert(std::is_same_v<U, OptType>, "Invalid option type");

        settings_.template get<static_cast<size_t>(idx)>() = std::forward<U>(value);
        event_table_.template Notify<static_cast<size_t>(idx)>();
    }

    template <AccessT idx>
    FORCE_INLINE_F void RegisterEvent(EventT &&event)
    {
        event_table_.template RegisterEvent<static_cast<size_t>(idx)>(std::move(event));
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    TupleT settings_;
    StaticEventTable<TypeListT::kSize, EventT> event_table_{};
};

// ------------------------------
// StaticPlainMap
// ------------------------------

template <class ItemT, size_t kSize, class KeyT = size_t>
    requires(
        std::convertible_to<KeyT, size_t> && std::is_trivially_copyable_v<ItemT> &&
        std::is_default_constructible_v<ItemT> && kSize > 0 &&
        kSize < 256  // Static maps should not be too large to avoid excessive memory usage
    )
class StaticPlainMapDefaultConstructible
{
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    constexpr StaticPlainMapDefaultConstructible()  = default;
    constexpr ~StaticPlainMapDefaultConstructible() = default;

    constexpr StaticPlainMapDefaultConstructible(const StaticPlainMapDefaultConstructible &) =
        default;
    constexpr StaticPlainMapDefaultConstructible(StaticPlainMapDefaultConstructible &&) = default;

    constexpr StaticPlainMapDefaultConstructible &operator=(
        const StaticPlainMapDefaultConstructible &
    ) = default;
    constexpr StaticPlainMapDefaultConstructible &operator=(StaticPlainMapDefaultConstructible &&) =
        default;

    // ------------------------------
    // Class methods
    // ------------------------------

    FORCE_INLINE_F constexpr ItemT &operator[](const KeyT &key) noexcept
    {
        const size_t idx = static_cast<size_t>(key);

        ASSERT_LT(idx, kSize, "Index out of range");
        return map_[idx];
    }

    FORCE_INLINE_F constexpr const ItemT &operator[](const KeyT &key) const noexcept
    {
        const size_t idx = static_cast<size_t>(key);

        ASSERT_LT(idx, kSize, "Index out of range");
        return map_[idx];
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    ItemT map_[kSize]{};
};

// ------------------------------
// EnumMap
// ------------------------------

template <class ItemT, class EnumT>
    requires std::is_enum_v<EnumT>
using EnumMap = StaticPlainMapDefaultConstructible<ItemT, static_cast<size_t>(EnumT::kLast), EnumT>;

// ------------------------------
// EnumFlagMap
// ------------------------------

template <class EnumT>
    requires std::is_enum_v<EnumT>
using EnumFlagMap =
    StaticPlainMapDefaultConstructible<bool, static_cast<size_t>(EnumT::kLast), EnumT>;

// ------------------------------
// ArrayStaticStack
// ------------------------------

template <size_t kSizeBytes, size_t kAlignment = 8>
class ArrayStaticStack
{
    // ------------------------------
    // Class creation
    // ------------------------------

    template <class T>
    using clean_type = std::remove_reference_t<std::remove_const_t<T>>;

    public:
    ArrayStaticStack()  = default;
    ~ArrayStaticStack() = default;

    ArrayStaticStack(const ArrayStaticStack &) = default;
    ArrayStaticStack(ArrayStaticStack &&)      = default;

    ArrayStaticStack &operator=(const ArrayStaticStack &) = default;
    ArrayStaticStack &operator=(ArrayStaticStack &&)      = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template <class T>
    FORCE_INLINE_F T &Push(const T &item)
    {
        auto ptr = PushMem_<T>();
        new (ptr) clean_type<T>(item); /* Checks alignment on debug */
        return *ptr;
    }

    template <class T>
    FORCE_INLINE_F T &Push(T &&item)
    {
        auto ptr = PushMem_<T>();
        new (ptr) clean_type<T>(std::move(item)); /* Checks alignment on debug */
        return *ptr;
    }

    template <class T, class... Args>
    FORCE_INLINE_F T &PushEmplace(Args &&...args)
    {
        auto ptr = PushMem_<T>();
        new (ptr) clean_type<T>(std::forward<Args>(args)...); /* Checks alignment on debug */
        return *ptr;
    }

    template <class T>
    FORCE_INLINE_F std::remove_reference_t<std::remove_const_t<T>> Pop()
    {
        ASSERT_LE(sizeof(T), top_, "Stack underflow!!");
        top_ -= sizeof(T);

        auto ptr = reinterpret_cast<clean_type<T> *>(stack_.data + top_);
        return std::move(*ptr);
    }

    NODISCARD FORCE_INLINE_F size_t Size() const { return top_; }

    NODISCARD FORCE_INLINE_F const void *Data() const { return stack_.data; }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    template <class T>
    clean_type<T> *PushMem_() noexcept
    {
        const size_t start = top_;
        top_ += sizeof(T);

        ASSERT_LE(top_, kSizeBytes, "Stack overflow!");

        return reinterpret_cast<clean_type<T> *>(stack_.data + start);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
    size_t top_{};
    std::aligned_storage_t<kSizeBytes, kAlignment> stack_{};
};

// ------------------------------
// ArraySingleTypeStaticStack
// ------------------------------

template <class T, size_t kNumObjects>
class ArraySingleTypeStaticStack : public ArrayStaticStack<sizeof(T) * kNumObjects, alignof(T)>
{
    using base_t = ArrayStaticStack<sizeof(T) * kNumObjects, alignof(T)>;
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    ArraySingleTypeStaticStack()  = default;
    ~ArraySingleTypeStaticStack() = default;

    ArraySingleTypeStaticStack(const ArraySingleTypeStaticStack &) = default;
    ArraySingleTypeStaticStack(ArraySingleTypeStaticStack &&)      = default;

    ArraySingleTypeStaticStack &operator=(const ArraySingleTypeStaticStack &) = default;
    ArraySingleTypeStaticStack &operator=(ArraySingleTypeStaticStack &&)      = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    FORCE_INLINE_F T &Push(const T &item) { return base_t::Push(item); }

    FORCE_INLINE_F T &Push(T &&item) { return base_t::Push(std::move(item)); }

    template <class... Args>
    FORCE_INLINE_F T &PushEmplace(Args &&...args)
    {
        return base_t::template PushEmplace<T>(std::forward<Args>(args)...);
    }

    FORCE_INLINE_F T Pop() { return std::move(base_t::template Pop<T>()); }

    FORCE_INLINE_F size_t Size() const { return base_t::Size() / sizeof(T); }

    FORCE_INLINE_F size_t SizeBytes() const { return base_t::Size(); }

    using base_t::Data;
};

// ------------------------------
// Static Vector
// ------------------------------

template <class T, size_t kMaxObjects>
class StaticVector : public ArraySingleTypeStaticStack<T, kMaxObjects>
{
    using base_t = ArraySingleTypeStaticStack<T, kMaxObjects>;

    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    StaticVector()  = default;
    ~StaticVector() = default;

    StaticVector(const StaticVector &) = default;
    StaticVector(StaticVector &&)      = default;

    StaticVector &operator=(const StaticVector &) = default;
    StaticVector &operator=(StaticVector &&)      = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    using base_t::Pop;
    using base_t::Push;
    using base_t::PushEmplace;
    using base_t::Size;

    // Iterator support
    FORCE_INLINE_F T *begin() { return reinterpret_cast<T *>(this->stack_.data); }
    FORCE_INLINE_F T *end() { return reinterpret_cast<T *>(this->stack_.data) + Size(); }

    FORCE_INLINE_F const T *cbegin() const
    {
        return reinterpret_cast<const T *>(this->stack_.data);
    }
    FORCE_INLINE_F const T *cend() const
    {
        return reinterpret_cast<const T *>(this->stack_.data) + Size();
    }

    FORCE_INLINE_F T &operator[](const size_t idx)
    {
        ASSERT_LT(idx, Size(), "Overflow detected!");
        return (begin()[idx]);
    }

    FORCE_INLINE_F const T &operator[](const size_t idx) const
    {
        ASSERT_LT(idx, Size(), "Overflow detected!");
        return (begin()[idx]);
    }

    void Resize(const size_t size)
    {
        ASSERT_LE(size, kMaxObjects, "Too many objects requested!!");

        if (size == Size()) {
            return;
        }

        if (size < Size()) {
            for (auto iter = begin() + size; iter != end(); ++iter) {
                iter->~T();
            }
        } else {
            for (auto iter = begin() + Size(); iter != begin() + size; ++iter) {
                new (iter) T();
            }
        }
        this->top_ = size * sizeof(T);
    }
};

//==============================================================================
// DelayedInitMixin
//==============================================================================

template <class T>
concept DelayedInitDerivedT = requires(T t) {
    { t.InitImpl() } -> std::same_as<void>;
};

template <class DerivedT, class ConfigT>
class DelayedInitMixin
{
    public:
    //==============================================================================
    // Class Construction
    //==============================================================================

    DelayedInitMixin() = default;
    DelayedInitMixin(const ConfigT &c) { Configure(c); }
    ~DelayedInitMixin() = default;

    DelayedInitMixin(const DelayedInitMixin &)            = delete;
    DelayedInitMixin &operator=(const DelayedInitMixin &) = delete;
    DelayedInitMixin(DelayedInitMixin &&)                 = delete;
    DelayedInitMixin &operator=(DelayedInitMixin &&)      = delete;

    //==============================================================================
    // Public Methods
    //==============================================================================

    void Configure(const ConfigT &c)
    {
        R_ASSERT_FALSE(c_.has_value());
        c_.emplace(c);
    }

    void Init();

    //==============================================================================
    // Observers
    //==============================================================================

    NODISCARD const ConfigT &GetConfig() const
    {
        R_ASSERT_TRUE(c_.has_value(), "Accessing config before configured.");
        return *c_;
    }
    NODISCARD bool IsConfigured() const { return c_.has_value(); }
    NODISCARD bool IsInitialized() const { return is_initialized_; }

    //==============================================================================
    // Private Fields
    //==============================================================================

    private:
    std::optional<ConfigT> c_;
    bool is_initialized_{false};
};

template <class DerivedT, class ConfigT>
void DelayedInitMixin<DerivedT, ConfigT>::Init()
{
    static_assert(
        DelayedInitDerivedT<DerivedT>,
        "DelayedInitMixin requires the derived type to have a 'void InitImpl()' method."
    );
    R_ASSERT_TRUE(c_.has_value());
    R_ASSERT_FALSE(is_initialized_);

    static_cast<DerivedT *>(this)->InitImpl();

    is_initialized_ = true;
}

}  // namespace template_lib
#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_TEMPLATE_LIB_HPP_
