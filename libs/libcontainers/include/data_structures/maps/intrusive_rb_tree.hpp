#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_MAPS_INTRUSIVE_RB_TREE_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_MAPS_INTRUSIVE_RB_TREE_HPP_

#include <assert.h>
#include <types.h>
#include <concepts.hpp>
#include <data_structures/intrusive_linked_list.hpp>
#include <defines.hpp>
#include <template/special_members.hpp>

namespace data_structures
{
template <class T, class KeyT, int kIntrusiveLevel>
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
static_assert(sizeof(IntrusiveRbNode<u64, u64, 0>) == 5 * 8);

template <class T, class KeyT, int kIntrusiveLevel>
    requires(
        std::derived_from<T, IntrusiveRbNode<T, KeyT, kIntrusiveLevel>> and
        std::derived_from<T, IntrusiveListNode<T>>
    )
class IntrusiveRBTree : template_lib::NoCopy
{
    using Color = typename IntrusiveRbNode<T, KeyT, kIntrusiveLevel>::Color;

    public:
    using HookT = IntrusiveRbNode<T, KeyT, kIntrusiveLevel>;

    // ------------------------------
    // Class creation
    // ------------------------------

    IntrusiveRBTree()  = default;
    ~IntrusiveRBTree() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F T *Root() { return root_; }

    NODISCARD FORCE_INLINE_F bool IsEmpty() { return root_ == nullptr; }

    void Insert(T *item)
    {
        ASSERT_NOT_NULL(item);

        item->HookT::left   = nullptr;
        item->HookT::right  = nullptr;
        item->HookT::parent = nullptr;
        item->HookT::color  = Color::kRed;
        item->next          = nullptr;

        // 1. Check if key already exists
        T *existing = Find(item->HookT::key);
        if (existing != nullptr) {
            item->next     = existing->next;
            existing->next = item;
            return;
        }

        // 2. Standard BST Insert
        T *y = nullptr;
        T *x = root_;

        while (x != nullptr) {
            y = x;
            if (item->HookT::key < x->HookT::key) {
                x = x->HookT::left;
            } else {
                x = x->HookT::right;
            }
        }

        item->HookT::parent = y;

        if (y == nullptr) {
            root_ = item;
        } else if (item->HookT::key < y->HookT::key) {
            y->HookT::left = item;
        } else {
            y->HookT::right = item;
        }

        if (min_ == nullptr || item->HookT::key < min_->HookT::key) {
            min_ = item;
        }
        if (max_ == nullptr || item->HookT::key > max_->HookT::key) {
            max_ = item;
        }

        InsertFixup_(item);
    }

    void Delete(T *item)
    {
        ASSERT_NOT_NULL(item);

        // Case A: The item is NOT part of the RB tree structure,

        T *node_in_tree = Find(item->HookT::key);
        if (node_in_tree == nullptr) {
            return;
        }

        if (node_in_tree != item) {
            T *prev = node_in_tree;
            while (prev->next != nullptr && prev->next != item) {
                prev = prev->next;
            }

            if (prev->next == item) {
                prev->next = item->next;
                item->next = nullptr;
            }
            return;
        }

        // Case B: item IS the node in the RB Tree.
        if (item->next != nullptr) {
            ReplaceWithNext_(item);
            return;
        }

        RBDelete_(item);
    }

    FORCE_INLINE_F void Delete(KeyT key)
    {
        T *node = Find(key);
        if (node == nullptr) {
            return;
        }

        RBDelete_(node);
    }

    NODISCARD bool Contains(T *item)
    {
        ASSERT_NOT_NULL(item);

        T *found = Find(item->HookT::key);
        if (!found) {
            return false;
        }

        T *curr = found;
        while (curr) {
            if (curr == item) {
                return true;
            }
            curr = curr->next;
        }
        return false;
    }

    NODISCARD FORCE_INLINE_F bool Contains(KeyT key) { return Find(key) != nullptr; }

    NODISCARD FORCE_INLINE_F T *Min() { return min_; }

    NODISCARD FORCE_INLINE_F T *Max() { return max_; }

    NODISCARD FORCE_INLINE_F T *Find(KeyT key)
    {
        T *current = root_;
        while (current != nullptr) {
            if (key == current->HookT::key) {
                return current;
            } else if (key < current->HookT::key) {
                current = current->HookT::left;
            } else {
                current = current->HookT::right;
            }
        }
        return nullptr;
    }

    NODISCARD FORCE_INLINE_F T *DeleteMin()
    {
        T *min_node = Min();
        Delete(min_node);
        return min_node;
    }

    NODISCARD FORCE_INLINE_F T *DeleteMax()
    {
        T *max_node = Max();
        Delete(max_node);
        return max_node;
    }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    void RecalculateMinMax_()
    {
        if (!root_) {
            min_ = nullptr;
            max_ = nullptr;
            return;
        }

        T *curr = root_;
        while (curr->HookT::left) curr = curr->HookT::left;
        min_ = curr;

        curr = root_;
        while (curr->HookT::right) curr = curr->HookT::right;
        max_ = curr;
    }

    void ReplaceWithNext_(T *item)
    {
        T *replacement = item->next;
        ASSERT_NOT_NULL(replacement);

        // Copy RB properties
        replacement->HookT::parent = item->HookT::parent;
        replacement->HookT::left   = item->HookT::left;
        replacement->HookT::right  = item->HookT::right;
        replacement->HookT::color  = item->HookT::color;

        // Update parent's child pointer
        if (item->HookT::parent == nullptr) {
            root_ = replacement;
        } else if (item == item->HookT::parent->HookT::left) {
            item->HookT::parent->HookT::left = replacement;
        } else {
            item->HookT::parent->HookT::right = replacement;
        }

        // Update children's parent pointer
        if (item->HookT::left) {
            item->HookT::left->HookT::parent = replacement;
        }
        if (item->HookT::right) {
            item->HookT::right->HookT::parent = replacement;
        }

        // Update Cached Min/Max if necessary
        if (min_ == item) {
            min_ = replacement;
        }
        if (max_ == item) {
            max_ = replacement;
        }

        // Clear item's pointers
        item->HookT::parent = nullptr;
        item->HookT::left   = nullptr;
        item->HookT::right  = nullptr;
        item->next          = nullptr;
    }

    void RotateLeft_(T *x)
    {
        T *y            = x->HookT::right;
        x->HookT::right = y->HookT::left;

        if (y->HookT::left != nullptr) {
            y->HookT::left->HookT::parent = x;
        }

        y->HookT::parent = x->HookT::parent;

        if (x->HookT::parent == nullptr) {
            root_ = y;
        } else if (x == x->HookT::parent->HookT::left) {
            x->HookT::parent->HookT::left = y;
        } else {
            x->HookT::parent->HookT::right = y;
        }

        y->HookT::left   = x;
        x->HookT::parent = y;
    }

    void RotateRight_(T *x)
    {
        T *y           = x->HookT::left;
        x->HookT::left = y->HookT::right;

        if (y->HookT::right != nullptr) {
            y->HookT::right->HookT::parent = x;
        }

        y->HookT::parent = x->HookT::parent;

        if (x->HookT::parent == nullptr) {
            root_ = y;
        } else if (x == x->HookT::parent->HookT::right) {
            x->HookT::parent->HookT::right = y;
        } else {
            x->HookT::parent->HookT::left = y;
        }

        y->HookT::right  = x;
        x->HookT::parent = y;
    }

    void InsertFixup_(T *z)
    {
        while (z->HookT::parent != nullptr && z->HookT::parent->HookT::color == Color::kRed) {
            if (z->HookT::parent == z->HookT::parent->HookT::parent->HookT::left) {
                T *y = z->HookT::parent->HookT::parent->HookT::right;  // Uncle

                if (y != nullptr && y->HookT::color == Color::kRed) {
                    // Case 1: Uncle is Red
                    z->HookT::parent->HookT::color                = Color::kBlack;
                    y->HookT::color                               = Color::kBlack;
                    z->HookT::parent->HookT::parent->HookT::color = Color::kRed;
                    z                                             = z->HookT::parent->HookT::parent;
                } else {
                    // Case 2: Uncle is Black (Triangle)
                    if (z == z->HookT::parent->HookT::right) {
                        z = z->HookT::parent;
                        RotateLeft_(z);
                    }
                    // Case 3: Uncle is Black (Line)
                    z->HookT::parent->HookT::color                = Color::kBlack;
                    z->HookT::parent->HookT::parent->HookT::color = Color::kRed;
                    RotateRight_(z->HookT::parent->HookT::parent);
                }
            } else {
                // Symmetric to above
                T *y = z->HookT::parent->HookT::parent->HookT::left;

                if (y != nullptr && y->HookT::color == Color::kRed) {
                    z->HookT::parent->HookT::color                = Color::kBlack;
                    y->HookT::color                               = Color::kBlack;
                    z->HookT::parent->HookT::parent->HookT::color = Color::kRed;
                    z                                             = z->HookT::parent->HookT::parent;
                } else {
                    if (z == z->HookT::parent->HookT::left) {
                        z = z->HookT::parent;
                        RotateRight_(z);
                    }
                    z->HookT::parent->HookT::color                = Color::kBlack;
                    z->HookT::parent->HookT::parent->HookT::color = Color::kRed;
                    RotateLeft_(z->HookT::parent->HookT::parent);
                }
            }
        }
        root_->HookT::color = Color::kBlack;
    }

    void RBTransplant_(T *u, T *v)
    {
        if (u->HookT::parent == nullptr) {
            root_ = v;
        } else if (u == u->HookT::parent->HookT::left) {
            u->HookT::parent->HookT::left = v;
        } else {
            u->HookT::parent->HookT::right = v;
        }
        if (v != nullptr) {
            v->HookT::parent = u->HookT::parent;
        }
    }

    T *TreeMinimum_(T *node)
    {
        while (node->HookT::left != nullptr) {
            node = node->HookT::left;
        }
        return node;
    }

    void RBDelete_(T *z)
    {
        T *y                   = z;
        T *x                   = nullptr;
        Color y_original_color = y->HookT::color;

        T *x_parent = nullptr;

        if (z->HookT::left == nullptr) {
            x        = z->HookT::right;
            x_parent = z->HookT::parent;
            RBTransplant_(z, z->HookT::right);
        } else if (z->HookT::right == nullptr) {
            x        = z->HookT::left;
            x_parent = z->HookT::parent;
            RBTransplant_(z, z->HookT::left);
        } else {
            y                = TreeMinimum_(z->HookT::right);
            y_original_color = y->HookT::color;
            x                = y->HookT::right;

            if (y->HookT::parent == z) {
                x_parent = y;
            } else {
                x_parent = y->HookT::parent;
                RBTransplant_(y, y->HookT::right);
                y->HookT::right                = z->HookT::right;
                y->HookT::right->HookT::parent = y;
            }

            RBTransplant_(z, y);
            y->HookT::left                = z->HookT::left;
            y->HookT::left->HookT::parent = y;
            y->HookT::color               = z->HookT::color;
        }

        if (y_original_color == Color::kBlack) {
            DeleteFixup_(x, x_parent);
        }

        z->HookT::parent = nullptr;
        z->HookT::left   = nullptr;
        z->HookT::right  = nullptr;

        if (z == min_ || z == max_) {
            RecalculateMinMax_();
        }
    }

    void DeleteFixup_(T *x, T *x_parent)
    {
        while (x != root_ && (x == nullptr || x->HookT::color == Color::kBlack)) {
            if (x == (x_parent ? x_parent->HookT::left : nullptr)) {  // x is left child
                T *w = x_parent->HookT::right;                        // Sibling

                if (w->HookT::color == Color::kRed) {
                    w->HookT::color        = Color::kBlack;
                    x_parent->HookT::color = Color::kRed;
                    RotateLeft_(x_parent);
                    w = x_parent->HookT::right;
                }

                if ((w->HookT::left == nullptr || w->HookT::left->HookT::color == Color::kBlack) &&
                    (w->HookT::right == nullptr ||
                     w->HookT::right->HookT::color == Color::kBlack)) {
                    w->HookT::color = Color::kRed;
                    x               = x_parent;
                    x_parent        = x ? x->HookT::parent : nullptr;
                } else {
                    if (w->HookT::right == nullptr ||
                        w->HookT::right->HookT::color == Color::kBlack) {
                        if (w->HookT::left)
                            w->HookT::left->HookT::color = Color::kBlack;
                        w->HookT::color = Color::kRed;
                        RotateRight_(w);
                        w = x_parent->HookT::right;
                    }
                    w->HookT::color        = x_parent->HookT::color;
                    x_parent->HookT::color = Color::kBlack;
                    if (w->HookT::right)
                        w->HookT::right->HookT::color = Color::kBlack;
                    RotateLeft_(x_parent);
                    x        = root_;
                    x_parent = nullptr;
                }
            } else {  // x is right child (symmetric)
                T *w = x_parent->HookT::left;

                if (w->HookT::color == Color::kRed) {
                    w->HookT::color        = Color::kBlack;
                    x_parent->HookT::color = Color::kRed;
                    RotateRight_(x_parent);
                    w = x_parent->HookT::left;
                }

                if ((w->HookT::right == nullptr ||
                     w->HookT::right->HookT::color == Color::kBlack) &&
                    (w->HookT::left == nullptr || w->HookT::left->HookT::color == Color::kBlack)) {
                    w->HookT::color = Color::kRed;
                    x               = x_parent;
                    x_parent        = x ? x->HookT::parent : nullptr;
                } else {
                    if (w->HookT::left == nullptr ||
                        w->HookT::left->HookT::color == Color::kBlack) {
                        if (w->HookT::right)
                            w->HookT::right->HookT::color = Color::kBlack;
                        w->HookT::color = Color::kRed;
                        RotateLeft_(w);
                        w = x_parent->HookT::left;
                    }
                    w->HookT::color        = x_parent->HookT::color;
                    x_parent->HookT::color = Color::kBlack;
                    if (w->HookT::left)
                        w->HookT::left->HookT::color = Color::kBlack;
                    RotateRight_(x_parent);
                    x        = root_;
                    x_parent = nullptr;
                }
            }
        }
        if (x != nullptr) {
            x->HookT::color = Color::kBlack;
        }
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    T *root_{};
    T *min_{};
    T *max_{};
};
}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_MAPS_INTRUSIVE_RB_TREE_HPP_
