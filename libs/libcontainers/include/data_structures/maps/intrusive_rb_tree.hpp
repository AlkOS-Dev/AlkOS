#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_MAPS_INTRUSIVE_RB_TREE_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_MAPS_INTRUSIVE_RB_TREE_HPP_

#include <types.h>
#include <concepts.hpp>
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
    requires std::derived_from<T, IntrusiveRbNode<T, KeyT>>
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

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    // ------------------------------
    // Class fields
    // ------------------------------

    T *root{};
};
}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_MAPS_INTRUSIVE_RB_TREE_HPP_
