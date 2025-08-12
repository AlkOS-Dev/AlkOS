#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_EVENTS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_EVENTS_HPP_

#include <extensions/template_lib.hpp>

namespace data_structures
{

// ------------------------------
// EventTable
// ------------------------------

template <size_t kSize, class EvenT>
class StaticEventTable : template_lib::MoveOnly
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
    requires template_lib::IsTypeList_v<TypeListT> &&
             requires { static_cast<size_t>(std::declval<AccessT>()); }
class Settings : public template_lib::NoCopy
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

}  // namespace data_structures

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_EVENTS_HPP_
