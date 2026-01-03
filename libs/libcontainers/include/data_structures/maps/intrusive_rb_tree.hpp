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
    using Color = typename IntrusiveRbNode<T, KeyT>::Color;

    public:
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

        item->left   = nullptr;
        item->right  = nullptr;
        item->parent = nullptr;
        item->color  = Color::kRed;
        item->next   = nullptr;

        // 1. Check if key already exists
        T *existing = Find(item->key);
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
            if (item->key < x->key) {
                x = x->left;
            } else {
                x = x->right;
            }
        }

        item->parent = y;

        if (y == nullptr) {
            root_ = item;
        } else if (item->key < y->key) {
            y->left = item;
        } else {
            y->right = item;
        }

        if (min_ == nullptr || item->key < min_->key) {
            min_ = item;
        }
        if (max_ == nullptr || item->key > max_->key) {
            max_ = item;
        }

        InsertFixup_(item);
    }

    void Delete(T *item)
    {
        ASSERT_NOT_NULL(item);

        // Case A: The item is NOT part of the RB tree structure,

        T *node_in_tree = Find(item->key);
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

        T *found = Find(item->key);
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
            if (key == current->key) {
                return current;
            } else if (key < current->key) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        return nullptr;
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
        while (curr->left) curr = curr->left;
        min_ = curr;

        curr = root_;
        while (curr->right) curr = curr->right;
        max_ = curr;
    }

    void ReplaceWithNext_(T *item)
    {
        T *replacement = item->next;
        ASSERT_NOT_NULL(replacement);

        // Copy RB properties
        replacement->parent = item->parent;
        replacement->left   = item->left;
        replacement->right  = item->right;
        replacement->color  = item->color;

        // Update parent's child pointer
        if (item->parent == nullptr) {
            root_ = replacement;
        } else if (item == item->parent->left) {
            item->parent->left = replacement;
        } else {
            item->parent->right = replacement;
        }

        // Update children's parent pointer
        if (item->left) {
            item->left->parent = replacement;
        }
        if (item->right) {
            item->right->parent = replacement;
        }

        // Update Cached Min/Max if necessary
        if (min_ == item) {
            min_ = replacement;
        }
        if (max_ == item) {
            max_ = replacement;
        }

        // Clear item's pointers
        item->parent = nullptr;
        item->left   = nullptr;
        item->right  = nullptr;
        item->next   = nullptr;
    }

    void RotateLeft_(T *x)
    {
        T *y     = x->right;
        x->right = y->left;

        if (y->left != nullptr) {
            y->left->parent = x;
        }

        y->parent = x->parent;

        if (x->parent == nullptr) {
            root_ = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }

        y->left   = x;
        x->parent = y;
    }

    void RotateRight_(T *x)
    {
        T *y    = x->left;
        x->left = y->right;

        if (y->right != nullptr) {
            y->right->parent = x;
        }

        y->parent = x->parent;

        if (x->parent == nullptr) {
            root_ = y;
        } else if (x == x->parent->right) {
            x->parent->right = y;
        } else {
            x->parent->left = y;
        }

        y->right  = x;
        x->parent = y;
    }

    void InsertFixup_(T *z)
    {
        while (z->parent != nullptr && z->parent->color == Color::kRed) {
            if (z->parent == z->parent->parent->left) {
                T *y = z->parent->parent->right;  // Uncle

                if (y != nullptr && y->color == Color::kRed) {
                    // Case 1: Uncle is Red
                    z->parent->color         = Color::kBlack;
                    y->color                 = Color::kBlack;
                    z->parent->parent->color = Color::kRed;
                    z                        = z->parent->parent;
                } else {
                    // Case 2: Uncle is Black (Triangle)
                    if (z == z->parent->right) {
                        z = z->parent;
                        RotateLeft_(z);
                    }
                    // Case 3: Uncle is Black (Line)
                    z->parent->color         = Color::kBlack;
                    z->parent->parent->color = Color::kRed;
                    RotateRight_(z->parent->parent);
                }
            } else {
                // Symmetric to above
                T *y = z->parent->parent->left;

                if (y != nullptr && y->color == Color::kRed) {
                    z->parent->color         = Color::kBlack;
                    y->color                 = Color::kBlack;
                    z->parent->parent->color = Color::kRed;
                    z                        = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        RotateRight_(z);
                    }
                    z->parent->color         = Color::kBlack;
                    z->parent->parent->color = Color::kRed;
                    RotateLeft_(z->parent->parent);
                }
            }
        }
        root_->color = Color::kBlack;
    }

    void RBTransplant_(T *u, T *v)
    {
        if (u->parent == nullptr) {
            root_ = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        if (v != nullptr) {
            v->parent = u->parent;
        }
    }

    T *TreeMinimum_(T *node)
    {
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    void RBDelete_(T *z)
    {
        T *y                   = z;
        T *x                   = nullptr;
        Color y_original_color = y->color;

        T *x_parent = nullptr;

        if (z->left == nullptr) {
            x        = z->right;
            x_parent = z->parent;
            RBTransplant_(z, z->right);
        } else if (z->right == nullptr) {
            x        = z->left;
            x_parent = z->parent;
            RBTransplant_(z, z->left);
        } else {
            y                = TreeMinimum_(z->right);
            y_original_color = y->color;
            x                = y->right;

            if (y->parent == z) {
                x_parent = y;
            } else {
                x_parent = y->parent;
                RBTransplant_(y, y->right);
                y->right         = z->right;
                y->right->parent = y;
            }

            RBTransplant_(z, y);
            y->left         = z->left;
            y->left->parent = y;
            y->color        = z->color;
        }

        if (y_original_color == Color::kBlack) {
            DeleteFixup_(x, x_parent);
        }

        z->parent = nullptr;
        z->left   = nullptr;
        z->right  = nullptr;

        if (z == min_ || z == max_) {
            RecalculateMinMax_();
        }
    }

    void DeleteFixup_(T *x, T *x_parent)
    {
        while (x != root_ && (x == nullptr || x->color == Color::kBlack)) {
            if (x == (x_parent ? x_parent->left : nullptr)) {  // x is left child
                T *w = x_parent->right;                        // Sibling

                if (w->color == Color::kRed) {
                    w->color        = Color::kBlack;
                    x_parent->color = Color::kRed;
                    RotateLeft_(x_parent);
                    w = x_parent->right;
                }

                if ((w->left == nullptr || w->left->color == Color::kBlack) &&
                    (w->right == nullptr || w->right->color == Color::kBlack)) {
                    w->color = Color::kRed;
                    x        = x_parent;
                    x_parent = x ? x->parent : nullptr;
                } else {
                    if (w->right == nullptr || w->right->color == Color::kBlack) {
                        if (w->left)
                            w->left->color = Color::kBlack;
                        w->color = Color::kRed;
                        RotateRight_(w);
                        w = x_parent->right;
                    }
                    w->color        = x_parent->color;
                    x_parent->color = Color::kBlack;
                    if (w->right)
                        w->right->color = Color::kBlack;
                    RotateLeft_(x_parent);
                    x        = root_;
                    x_parent = nullptr;
                }
            } else {  // x is right child (symmetric)
                T *w = x_parent->left;

                if (w->color == Color::kRed) {
                    w->color        = Color::kBlack;
                    x_parent->color = Color::kRed;
                    RotateRight_(x_parent);
                    w = x_parent->left;
                }

                if ((w->right == nullptr || w->right->color == Color::kBlack) &&
                    (w->left == nullptr || w->left->color == Color::kBlack)) {
                    w->color = Color::kRed;
                    x        = x_parent;
                    x_parent = x ? x->parent : nullptr;
                } else {
                    if (w->left == nullptr || w->left->color == Color::kBlack) {
                        if (w->right)
                            w->right->color = Color::kBlack;
                        w->color = Color::kRed;
                        RotateLeft_(w);
                        w = x_parent->left;
                    }
                    w->color        = x_parent->color;
                    x_parent->color = Color::kBlack;
                    if (w->left)
                        w->left->color = Color::kBlack;
                    RotateRight_(x_parent);
                    x        = root_;
                    x_parent = nullptr;
                }
            }
        }
        if (x != nullptr) {
            x->color = Color::kBlack;
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
