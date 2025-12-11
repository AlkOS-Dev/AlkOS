#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_LRU_CACHE_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_LRU_CACHE_HPP_

#include <stddef.h>
#include <type_traits.hpp>

#include <data_structures/hash_maps.hpp>
#include <data_structures/linked_list.hpp>

namespace data_structures
{

template <
    typename KeyT, typename ValueT, size_t kCapacity,
    class HashT = FastMinimalStaticHashmap<
        KeyT, internal::LinkedListNode<std::tuple<const KeyT, ValueT>> *, kCapacity>>
class LruCache
{
    static_assert(kCapacity > 0, "LRU cache capacity must be greater than zero");

    public:
    LruCache() = default;
    ~LruCache() { Clear(); }

    size_t Size() const { return list_.Size(); }
    static constexpr size_t Capacity() { return kCapacity; }
    bool Empty() const { return list_.Empty(); }

    bool Contains(const KeyT &key) const { return hash_map_.HasKey(key); }

    ValueT *Get(const KeyT &key)
    {
        auto node_ptr = hash_map_.Find(key);
        if (!node_ptr) {
            return nullptr;
        }

        ListNode *node = *node_ptr;
        list_.MoveToFront(node);
        return &std::get<1>(node->data);
    }

    const ValueT *Get(const KeyT &key) const
    {
        auto node_ptr = hash_map_.Find(key);
        if (!node_ptr) {
            return nullptr;
        }

        ListNode *node = *node_ptr;
        const_cast<decltype(list_) &>(list_).MoveToFront(node);
        return &std::get<1>(node->data);
    }

    void Put(const KeyT &key, ValueT value)
    {
        if (auto node_ptr = hash_map_.Find(key)) {
            ListNode *node          = *node_ptr;
            std::get<1>(node->data) = std::move(value);
            list_.MoveToFront(node);
            return;
        }

        if (list_.Size() == kCapacity) {
            Evict_();
        }

        if (ListNode *new_node = list_.EmplaceFront(key, std::move(value))) {
            hash_map_.Insert(std::get<0>(new_node->data), new_node);
        }
    }

    bool Erase(const KeyT &key)
    {
        auto node_ptr = hash_map_.Find(key);
        if (!node_ptr) {
            return false;
        }

        ListNode *node = *node_ptr;
        hash_map_.Remove(key);
        list_.Remove(node);
        return true;
    }

    void Clear()
    {
        list_.Clear();
        hash_map_ = HashT();
    }

    private:
    void Evict_()
    {
        ListNode *tail = list_.GetTail();
        if (!tail) {
            return;
        }

        hash_map_.Remove(std::get<0>(tail->data));
        list_.PopBack();
    }

    DoubleLinkedList<std::tuple<const KeyT, ValueT>> list_{};
    HashT hash_map_{};

    using ListNode = decltype(list_)::Node;
};

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_LRU_CACHE_HPP_
