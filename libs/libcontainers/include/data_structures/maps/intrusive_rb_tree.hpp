#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_MAPS_INTRUSIVE_RB_TREE_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_MAPS_INTRUSIVE_RB_TREE_HPP_

#include <types.h>
#include <concepts.hpp>
#include <data_structures/intrusive_linked_list.hpp>
#include <defines.hpp>
#include <template/special_members.hpp>

namespace data_structures
{
template <class T, class KeyT>
struct IntrusiveRbNode {
    enum class Color : u8 {
        kBlack = 0,
        kRed   = 1,
    };

    T *parent;
    union {
        struct {
            T *left;
            T *right;
        };
        T *child[2];
    };

    Color color;
    KeyT key;
};

template <class T, class KeyT>
    requires(
        std::derived_from<T, IntrusiveRbNode<T, KeyT>> and
        std::derived_from<T, IntrusiveListNode<T>>
    )
class IntrusiveRBTree : template_lib::NoCopy
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    IntrusiveRBTree()  = default;
    ~IntrusiveRBTree() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void Insert(T *item) {}

    void Delete(T *item) {}

    void Delete(KeyT key) {}

    void Contains(T *item) {}

    void Contains(KeyT key) {}

    NODISCARD T *Min() {}

    NODISCARD T *Max() {}

    NODISCARD T *Find(KeyT key) {}

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    // ------------------------------
    // Class fields
    // ------------------------------

    T *root_{};
    T *min_{};
    T *max_{};
};
}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_MAPS_INTRUSIVE_RB_TREE_HPP_
