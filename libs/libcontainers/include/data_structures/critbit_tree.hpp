#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_CRITBIT_TREE_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_CRITBIT_TREE_HPP_

#include <string.h>
#include <data_structures/bit_array.hpp>
#include <data_structures/tagged_pointer.hpp>
#include <mem/heap.hpp>
#include <new.hpp>
#include <optional.hpp>
#include <string.hpp>

namespace data_structures
{

// Crit-bit tree reference:
// https://github.com/agl/critbit/blob/4bb69901a9813de05331bd3afc5085e00050f701/critbit.pdf

/**
 * @brief A binary crit-bit tree implementation for NUL-terminated strings.
 *
 * This implementation uses a `TaggedPointer` to distinguish between
 * internal nodes and external nodes.
 *
 * @tparam T The type of value stored in the tree.
 */
template <typename T>
class CritBitTree
{
    // Forward declarations
    struct ExternalNode;
    struct InternalNode;

    // Tag 0: ExternalNode
    // Tag 1: InternalNode
    using NodePtr = OwningTaggedPtr<ExternalNode, InternalNode>;

    struct ExternalNode {
        char *key;
        T value;

        template <typename... Args>
        ExternalNode(const char *k, Args &&...args) : value(std::forward<Args>(args)...)
        {
            const size_t len = strlen(k);
            key              = static_cast<char *>(Mem::KMalloc(len + 1).value_or(nullptr));
            if (key) {
                strncpy(key, k, len + 1);
            }
        }

        ~ExternalNode()
        {
            if (key) {
                Mem::KFree(key);
            }
        }
    };

    struct InternalNode {
        NodePtr child[2];
        u32 byte;
        u8 mask;

        InternalNode(u32 b, u8 o) : byte(b), mask(o) {}
        ~InternalNode() = default;
    };

    public:
    using value_type    = T;
    using pointer       = value_type *;
    using const_pointer = const value_type *;

    // -----------------------------------------------------------------------------
    // Construction / Destruction
    // -----------------------------------------------------------------------------

    CritBitTree() = default;

    ~CritBitTree() { Clear(); }

    CritBitTree(const CritBitTree &)            = delete;
    CritBitTree &operator=(const CritBitTree &) = delete;

    CritBitTree(CritBitTree &&other) noexcept : root_(std::move(other.root_)) {}

    CritBitTree &operator=(CritBitTree &&other) noexcept
    {
        if (this != &other) {
            root_ = std::move(other.root_);
        }
        return *this;
    }

    // -----------------------------------------------------------------------------
    // Public API
    // -----------------------------------------------------------------------------

    void Clear() { root_ = {}; }

    /**
     * @brief Checks if a key exists in the tree.
     */
    NODISCARD FORCE_INLINE_F bool Contains(std::string_view key) const
    {
        const ExternalNode *match = GetBestMatch(key);
        if (!match) {
            return false;
        }

        return std::string_view(match->key) == key;
    }

    /**
     * @brief Retrieves a pointer to the value associated with the key.
     * @return An optional containing the pointer, or empty if not found.
     */
    NODISCARD FORCE_INLINE_F std::optional<pointer> Get(std::string_view key)
    {
        ExternalNode *match = GetBestMatch(key);
        if (!match) {
            return {};
        }

        if (std::string_view(match->key) != key) {
            return {};
        }

        auto ret = std::optional<pointer>();
        ret.emplace(&match->value);

        return ret;
    }

    /**
     * @brief Retrieves a const pointer to the value associated with the key.
     */
    NODISCARD FORCE_INLINE_F std::optional<const_pointer> Get(std::string_view key) const
    {
        const ExternalNode *match = GetBestMatch(key);
        if (!match) {
            return {};
        }

        if (std::string_view(match->key) != key) {
            return {};
        }

        auto ret = std::optional<const_pointer>();
        ret.emplace(&match->value);

        return ret;
    }

    NODISCARD FORCE_INLINE_F std::optional<pointer> operator[](std::string_view key)
    {
        return Get(key);
    }
    NODISCARD FORCE_INLINE_F std::optional<const_pointer> operator[](std::string_view key) const
    {
        return Get(key);
    }

    /**
     * @brief Get Longest Prefix Match for the given key.
     * @param key The key to search for.
     * @return An optional containing a pointer to the value, or empty if no match found.
     */
    NODISCARD FORCE_INLINE_F std::optional<pointer> GetLongestPrefixMatch(std::string_view key)
    {
        ExternalNode *match = GetBestMatch(key);
        if (!match) {
            return {};
        }

        auto ret = std::optional<pointer>();
        ret.emplace(&match->value);
        return ret;
    }

    /**
     * @brief Inserts a key-value pair into the tree.
     *
     * @param key A NUL-terminated string.
     * @param args Arguments to construct the value T.
     *
     * @tparam overwrite If true, overwrites the value if the key already exists.
     * @return true if inserted (or overwritten), false if key exists (and overwrite is false) or
     * allocation failed.
     */
    template <bool overwrite = false, typename... Args>
    bool Insert(const char *key, Args &&...args)
    {
        // 1. Handle insertion into an empty tree
        if (!root_.IsValid()) {
            root_ = NodePtr::template Construct<ExternalNode>(key, std::forward<Args>(args)...);
            if (!root_.IsValid()) {
                return false;
            }
            return true;
        }

        // 2. Walk tree for best member
        ExternalNode *best_match = GetBestMatch(key);

        // 3. Check for successful membership (exact match)
        // Find the first differing byte
        const char *p = best_match->key;
        size_t i      = 0;
        while (p[i] != '\0' && key[i] == p[i]) {
            i++;
        }

        // If we reached the end of both strings, it's a match
        if (key[i] == '\0' && p[i] == '\0') {
            if constexpr (overwrite) {
                // Update value in place
                best_match->value.~T();
                new (&best_match->value) T(std::forward<Args>(args)...);
                return true;
            }
            return false;
        }

        // 4. Find the critical bit
        u32 newbyte = static_cast<u32>(i);
        u32 newmask = 0;

        // Get the differing bytes
        u8 u_char = static_cast<u8>(key[newbyte]);
        u8 p_char = static_cast<u8>(p[newbyte]);

        newmask = u_char ^ p_char;

        // Find the most significant differing bit.
        newmask |= (newmask >> 1);
        newmask |= (newmask >> 2);
        newmask |= (newmask >> 4);
        newmask = (newmask & ~(newmask >> 1)) ^ 255;

        u8 mask = static_cast<u8>(newmask);

        // 5. Allocate new internal node and external node
        // Calculate direction for the new key at this critical bit
        int dir = (1 + (mask | p_char)) >> 8;

        NodePtr new_ext_node =
            NodePtr::template Construct<ExternalNode>(key, std::forward<Args>(args)...);
        if (!new_ext_node.IsValid()) {
            return false;
        }

        NodePtr new_int_node = NodePtr::template Construct<InternalNode>(newbyte, mask);
        if (!new_int_node.IsValid()) {
            return false;
        }

        // Link the new external node
        auto &internal_ref          = new_int_node.template As<InternalNode>();
        internal_ref.child[1 - dir] = std::move(new_ext_node);

        // 6. Insert new node into the tree
        InsertInternalNode(new_int_node, newbyte, mask, dir, key);

        return true;
    }

    /**
     * @brief Removes a key from the tree.
     */
    bool Remove(std::string_view key)
    {
        if (!root_.IsValid()) {
            return false;
        }

        NodePtr *current = &root_;
        NodePtr *parent  = nullptr;
        InternalNode *q  = nullptr;
        int dir          = 0;

        const char *u     = key.data();
        const size_t ulen = key.size();

        // Walk to find best match, keeping track of parent
        while (current->template Is<InternalNode>()) {
            parent = current;
            q      = &current->template As<InternalNode>();

            const u8 c = (q->byte < ulen) ? static_cast<u8>(u[q->byte]) : 0;
            dir        = (1 + (q->mask | c)) >> 8;
            current    = &q->child[dir];
        }

        ExternalNode &p = current->template As<ExternalNode>();
        if (std::string_view(p.key) != key) {
            return false;
        }

        // Found it. Remove p.
        // If it's the root (and it's external), the tree becomes empty.
        if (!parent) {
            root_ = {};
            return true;
        }

        *parent = NodePtr(q->child[1 - dir].Release());
        return true;
    }

    // -----------------------------------------------------------------------------
    // Private Helpers
    // -----------------------------------------------------------------------------

    private:
    ExternalNode *GetBestMatch(std::string_view key) const
    {
        if (!root_.IsValid()) {
            return nullptr;
        }

        const NodePtr *p  = &root_;
        const char *u     = key.data();
        const size_t ulen = key.size();

        while (p->template Is<InternalNode>()) {
            const InternalNode &q = p->template As<InternalNode>();
            const u8 c            = (q.byte < ulen) ? static_cast<u8>(u[q.byte]) : 0;
            int dir               = (1 + (q.mask | c)) >> 8;
            p                     = &q.child[dir];
        }

        // p is now an ExternalNode
        return &const_cast<NodePtr *>(p)->template As<ExternalNode>();
    }

    void InsertInternalNode(
        NodePtr &new_node_ptr, u32 newbyte, u8 newmask, int newdir, const char *key
    )
    {
        NodePtr *wherep      = &root_;
        const size_t key_len = strlen(key);

        // Walk the tree again to find the insertion point.
        // We insert above the first node that checks a bit position *after* our new critical bit.
        while (wherep->template Is<InternalNode>()) {
            InternalNode &q = wherep->template As<InternalNode>();
            if (q.byte > newbyte || (q.byte == newbyte && q.mask > newmask)) {
                break;
            }

            // Move down
            const u8 c    = (q.byte < key_len) ? static_cast<u8>(key[q.byte]) : 0;
            const int dir = (1 + (q.mask | c)) >> 8;
            wherep        = &q.child[dir];
        }

        // The other child of the new node adopts the subtree we are replacing
        auto &new_node         = new_node_ptr.template As<InternalNode>();
        new_node.child[newdir] = std::move(*wherep);

        // The parent points to the new node
        *wherep = std::move(new_node_ptr);
    }

    NodePtr root_;
};

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_CRITBIT_TREE_HPP_
