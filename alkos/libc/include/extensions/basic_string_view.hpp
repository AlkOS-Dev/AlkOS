#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_BASIC_STRING_VIEW_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_BASIC_STRING_VIEW_HPP_

#include <todo.h>
#include <extensions/algorithm.hpp>
#include <extensions/array.hpp>
#include <extensions/char_traits.hpp>
#include <extensions/concepts.hpp>
#include <extensions/utility.hpp>

namespace std
{

template <typename CharT, typename Traits = char_traits<CharT>>
class basic_string_view
{
    public:
    // ------------------------------
    // Member types
    // ------------------------------
    using traits_type     = Traits;
    using value_type      = CharT;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;
    using reference       = value_type &;
    using const_reference = const value_type &;
    using pointer         = value_type *;
    using const_pointer   = const value_type *;

    TODO_LIBCPP_COMPLIANCE
    using iterator               = value_type *;
    using const_iterator         = const value_type *;
    using reverse_iterator       = iterator;
    using const_reverse_iterator = const_iterator;

    static constexpr size_type npos = static_cast<size_type>(-1);

    // ------------------------------
    // Constructors and assignment
    // ------------------------------
    constexpr basic_string_view() noexcept                               = default;
    constexpr basic_string_view(const basic_string_view &other) noexcept = default;
    constexpr basic_string_view(const CharT *str, size_t len) noexcept : data_(str), size_(len) {}
    constexpr basic_string_view(const CharT *str) noexcept : data_(str), size_(Traits::length(str))
    {
    }
    template <class R>
        requires requires(R &&r) {
            { r.data() } -> std::convertible_to<const CharT *>;
            { r.size() } -> std::convertible_to<size_type>;
        }
    constexpr explicit basic_string_view(R &&r) : data_(r.data()), size_(r.size())
    {
    }

    basic_string_view(nullptr_t) = delete;

    constexpr basic_string_view &operator=(const basic_string_view &other) noexcept = default;

    // ------------------------------
    // Iterators
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr const_iterator begin() const noexcept { return data_; }
    NODISCARD FORCE_INLINE_F constexpr const_iterator cbegin() const noexcept { return data_; }

    NODISCARD FORCE_INLINE_F constexpr const_iterator end() const noexcept { return data_ + size_; }
    NODISCARD FORCE_INLINE_F constexpr const_iterator cend() const noexcept
    {
        return data_ + size_;
    }

    NODISCARD FORCE_INLINE_F constexpr const_reverse_iterator rbegin() const noexcept
    {
        return end();
    }
    NODISCARD FORCE_INLINE_F constexpr const_reverse_iterator crbegin() const noexcept
    {
        return end();
    }

    NODISCARD FORCE_INLINE_F constexpr const_reverse_iterator rend() const noexcept
    {
        return begin();
    }
    NODISCARD FORCE_INLINE_F constexpr const_reverse_iterator crend() const noexcept
    {
        return begin();
    }

    // ------------------------------
    // Element access
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr const_reference operator[](size_type pos) const noexcept
    {
        if constexpr (kIsKernel) {
            ASSERT_LT(pos, size_);
        }
        return data_[pos];
    }

    NODISCARD FORCE_INLINE_F constexpr const_reference at(size_type pos) const
    {
        if constexpr (kIsKernel) {
            R_ASSERT_LT(pos, size_);
        } else {
            TODO_USERSPACE
        }
        return data_[pos];
    }

    NODISCARD FORCE_INLINE_F constexpr const_reference front() const noexcept
    {
        if constexpr (kIsKernel) {
            ASSERT_GT(size_, 0);
        }
        return data_[0];
    }

    NODISCARD FORCE_INLINE_F constexpr const_reference back() const noexcept
    {
        if constexpr (kIsKernel) {
            ASSERT_GT(size_, 0);
        }
        return data_[size_ - 1];
    }

    NODISCARD FORCE_INLINE_F constexpr const_pointer data() const noexcept { return data_; }

    // ------------------------------
    // Capacity
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr size_type size() const noexcept { return size_; }
    NODISCARD FORCE_INLINE_F constexpr size_type length() const noexcept { return size_; }

    NODISCARD FORCE_INLINE_F constexpr size_type max_size() const noexcept
    {
        return SIZE_MAX / sizeof(CharT);
    }

    NODISCARD FORCE_INLINE_F constexpr bool empty() const noexcept { return size_ == 0; }

    // ------------------------------
    // Modifiers
    // ------------------------------

    FORCE_INLINE_F constexpr void remove_prefix(size_type n)
    {
        if constexpr (kIsKernel) {
            ASSERT_LE(n, size_);
        }
        data_ += n;
        size_ -= n;
    }

    FORCE_INLINE_F constexpr void remove_suffix(size_type n)
    {
        if constexpr (kIsKernel) {
            ASSERT_LE(n, size_);
        }
        size_ -= n;
    }

    FORCE_INLINE_F constexpr void swap(basic_string_view &other) noexcept
    {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
    }

    // ------------------------------
    // Operations
    // ------------------------------

    // ------------------------------
    // std::string_view::copy
    // ------------------------------
    NODISCARD FORCE_INLINE_F constexpr size_type copy(
        CharT *dest, size_type n, size_type pos = 0
    ) const
    {
        if constexpr (kIsKernel) {
            R_ASSERT_LE(pos, size_);
        } else {
            TODO_USERSPACE
        }

        auto count = std::min(n, size_ - pos);
        Traits::copy(dest, data_ + pos, count);
        return count;
    }

    // ------------------------------
    // std::string_view::substr
    // ------------------------------
    NODISCARD FORCE_INLINE_F constexpr basic_string_view substr(
        size_type pos = 0, size_type n = npos
    ) const
    {
        if constexpr (kIsKernel) {
            R_ASSERT_LE(pos, size_);
        } else {
            TODO_USERSPACE
        }

        auto count = std::min(n, size_ - pos);
        return basic_string_view(data_ + pos, count);
    }

    // ------------------------------
    // std::string_view::compare
    // ------------------------------
    NODISCARD FORCE_INLINE_F constexpr int compare(basic_string_view v) const noexcept
    {
        int result = Traits::compare(data_, v.data_, std::min(size_, v.size_));
        if (result != 0) {
            return result;
        } else {
            if (size_ < v.size_) {
                return -1;
            } else if (size_ > v.size_) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    NODISCARD FORCE_INLINE_F constexpr int compare(
        size_type pos1, size_type n1, basic_string_view v
    ) const
    {
        return substr(pos1, n1).compare(v);
    }

    NODISCARD FORCE_INLINE_F constexpr int compare(
        size_type pos1, size_type n1, basic_string_view v, size_type pos2, size_type n2
    ) const
    {
        return substr(pos1, n1).compare(v.substr(pos2, n2));
    }

    NODISCARD FORCE_INLINE_F constexpr int compare(const CharT *s) const
    {
        return compare(basic_string_view(s));
    }

    NODISCARD FORCE_INLINE_F constexpr int compare(
        size_type pos1, size_type n1, const CharT *s
    ) const
    {
        return substr(pos1, n1).compare(basic_string_view(s));
    }

    NODISCARD FORCE_INLINE_F constexpr int compare(
        size_type pos1, size_type n1, const CharT *s, size_type n2
    ) const
    {
        return substr(pos1, n1).compare(basic_string_view(s, n2));
    }

    // ------------------------------
    // std::string_view::starts_with
    // ------------------------------
    NODISCARD FORCE_INLINE_F constexpr bool starts_with(basic_string_view v) const noexcept
    {
        return basic_string_view(data_, std::min(size_, v.size_)) == v;
    }

    NODISCARD FORCE_INLINE_F constexpr bool starts_with(CharT c) const noexcept
    {
        return !empty() && Traits::eq(front(), c);
    }

    NODISCARD FORCE_INLINE_F constexpr bool starts_with(const CharT *s) const
    {
        return starts_with(basic_string_view(s));
    }

    // ------------------------------
    // std::string_view::ends_with
    // ------------------------------
    NODISCARD FORCE_INLINE_F constexpr bool ends_with(basic_string_view v) const noexcept
    {
        return size_ >= v.size_ && compare(size_ - v.size_, npos, v) == 0;
    }

    NODISCARD FORCE_INLINE_F constexpr bool ends_with(CharT c) const noexcept
    {
        return !empty() && Traits::eq(back(), c);
    }

    NODISCARD FORCE_INLINE_F constexpr bool ends_with(const CharT *s) const
    {
        return ends_with(basic_string_view(s));
    }

    // ------------------------------
    // std::string_view::constains
    // ------------------------------
    NODISCARD FORCE_INLINE_F constexpr bool contains(basic_string_view v) const noexcept
    {
        return find(v) != npos;
    }

    NODISCARD FORCE_INLINE_F constexpr bool contains(CharT c) const noexcept
    {
        return find(c) != npos;
    }

    NODISCARD FORCE_INLINE_F constexpr bool contains(const CharT *s) const
    {
        return find(s) != npos;
    }

    // ------------------------------
    // std::string_view::find
    // ------------------------------

    // Used Knuth-Morris-Pratt (KMP) algorithm for substring search
    NODISCARD FORCE_INLINE_F constexpr size_type find(
        basic_string_view v, size_type pos = 0
    ) const noexcept
    {
        if (v.empty()) {
            return pos < size_ ? pos : npos;
        }

        if (pos >= size_) {
            return npos;
        }

        auto prefix_table = compute_prefix_table_(v);
        size_type i       = pos;
        size_type j       = 0;

        while (i < size_) {
            if (v[j] == data_[i]) {
                i++;
                j++;
            }
            if (j == v.size_) {
                return i - j;  // Found the substring
            } else if (i < size_ && v[j] != data_[i]) {
                if (j != 0) {
                    j = prefix_table[j - 1];  // Use the prefix table to skip characters
                } else {
                    i++;
                }
            }
        }

        return npos;
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find(CharT c, size_type pos = 0) const noexcept
    {
        return find(basic_string_view(addressof(c), 1), pos);
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find(
        const CharT *s, size_type pos, size_type n
    ) const
    {
        return find(basic_string_view(s, n), pos);
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find(const CharT *s, size_type pos = 0) const
    {
        return find(basic_string_view(s), pos);
    }

    // ------------------------------
    // std::string_view::rfind
    // ------------------------------

    // Reverse KMP algorithm for substring search
    NODISCARD FORCE_INLINE_F constexpr size_type rfind(
        basic_string_view v, size_type pos = npos
    ) const noexcept
    {
        if (v.empty()) {
            return pos < size_ ? pos : npos;
        }
        if (size_ < v.size_) {
            return npos;
        }

        auto prefix_table = compute_prefix_table_(v);
        size_type i       = pos < size_ ? pos : size_ - 1;
        size_type j       = v.size_ - 1;

        while (i >= 0) {
            if (v[j] == data_[i]) {
                i--;
                j--;
            }
            if (j < 0) {
                return i + 1;  // Found the substring
            } else if (i >= 0 && v[j] != data_[i]) {
                if (j != v.size_ - 1) {
                    j = prefix_table[j];  // Use the prefix table to skip characters
                } else {
                    i--;
                }
            }
        }

        return npos;
    }

    NODISCARD FORCE_INLINE_F constexpr size_type rfind(CharT c, size_type pos = npos) const noexcept
    {
        return rfind(basic_string_view(addressof(c), 1), pos);
    }

    NODISCARD FORCE_INLINE_F constexpr size_type rfind(
        const CharT *s, size_type pos, size_type n
    ) const
    {
        return rfind(basic_string_view(s, n), pos);
    }

    NODISCARD FORCE_INLINE_F constexpr size_type rfind(const CharT *s, size_type pos = npos) const
    {
        return rfind(basic_string_view(s), pos);
    }

    // ------------------------------
    // std::string_view::find_first_of
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr size_type find_first_of(
        basic_string_view v, size_type pos = 0
    ) const noexcept
    {
        if (empty() || pos >= size_) {
            return npos;
        }

        for (size_type i = pos; i < size_; ++i) {
            if (v.find(data_[i]) != npos) {
                return i;
            }
        }
        return npos;
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find_first_of(
        CharT c, size_type pos = 0
    ) const noexcept
    {
        return find_first_of(basic_string_view(addressof(c), 1), pos);
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find_first_of(
        const CharT *s, size_type pos, size_type n
    ) const
    {
        return find_first_of(basic_string_view(s, n), pos);
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find_first_of(
        const CharT *s, size_type pos = 0
    ) const
    {
        return find_first_of(basic_string_view(s), pos);
    }

    // ------------------------------
    // std::string_view::find_last_of
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr size_type find_last_of(
        basic_string_view v, size_type pos = npos
    ) const noexcept
    {
        if (empty()) {
            return npos;
        }

        for (size_type i = std::min(pos, size_ - 1); i != npos; --i) {
            if (v.find(data_[i]) != npos) {
                return i;
            }
        }

        return npos;
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find_last_of(
        CharT c, size_type pos = npos
    ) const noexcept
    {
        return find_last_of(basic_string_view(addressof(c), 1), pos);
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find_last_of(
        const CharT *s, size_type pos, size_type n
    ) const
    {
        return find_last_of(basic_string_view(s, n), pos);
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find_last_of(
        const CharT *s, size_type pos = npos
    ) const
    {
        return find_last_of(basic_string_view(s), pos);
    }

    // ------------------------------
    // std::string_view::find_first_not_of
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr size_type find_first_not_of(
        basic_string_view v, size_type pos = 0
    ) const noexcept
    {
        if (empty() || pos >= size_) {
            return npos;
        }

        for (size_type i = pos; i < size_; ++i) {
            if (v.find(data_[i]) == npos) {
                return i;
            }
        }
        return npos;
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find_first_not_of(
        CharT c, size_type pos = 0
    ) const noexcept
    {
        return find_first_not_of(basic_string_view(addressof(c), 1), pos);
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find_first_not_of(
        const CharT *s, size_type pos, size_type n
    ) const
    {
        return find_first_not_of(basic_string_view(s, n), pos);
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find_first_not_of(
        const CharT *s, size_type pos = 0
    ) const
    {
        return find_first_not_of(basic_string_view(s), pos);
    }

    // ------------------------------
    // std::string_view::find_last_not_of
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr size_type find_last_not_of(
        basic_string_view v, size_type pos = npos
    ) const noexcept
    {
        if (empty()) {
            return npos;
        }

        for (size_type i = std::min(pos, size_ - 1); i != npos; --i) {
            if (v.find(data_[i]) == npos) {
                return i;
            }
        }

        return npos;
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find_last_not_of(
        CharT c, size_type pos = npos
    ) const noexcept
    {
        return find_last_not_of(basic_string_view(addressof(c), 1), pos);
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find_last_not_of(
        const CharT *s, size_type pos, size_type n
    ) const
    {
        return find_last_not_of(basic_string_view(s, n), pos);
    }

    NODISCARD FORCE_INLINE_F constexpr size_type find_last_not_of(
        const CharT *s, size_type pos = npos
    ) const
    {
        return find_last_not_of(basic_string_view(s), pos);
    }

    // ------------------------------
    // Helper functions
    // ------------------------------

    private:
    // Computes the prefix table for the Knuth-Morris-Pratt (KMP) algorithm.
    NODISCARD FORCE_INLINE_F constexpr auto compute_prefix_table_(basic_string_view v) const
    {
        constexpr size_type kMaxPrefixTableSize =
            4096;  // Arbitrary size limit for the prefix table
        array<size_type, kMaxPrefixTableSize> prefix_table{};
        size_type j = 0;
        for (size_type i = 1; i < v.size_; i++) {
            while (j > 0 && v[j] != v[i]) j = prefix_table[j - 1];

            if (v[j] == v[i])
                j++;

            prefix_table[i] = j;
        }
        return prefix_table;
    }

    // ------------------------------
    // Data members
    // ------------------------------

    private:
    const_pointer data_{};
    size_type size_{};
};

template <class CharT, class Traits>
constexpr bool operator==(
    basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs
) noexcept
{
    return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
}

template <class CharT, class Traits>
constexpr bool operator==(
    basic_string_view<CharT, Traits> lhs, type_identity_t<basic_string_view<CharT, Traits>> rhs
) noexcept
{
    return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
}

template <typename CharT, typename Traits>
NODISCARD constexpr Traits::comparison_category operator<=>(
    basic_string_view<CharT, Traits> lhs, type_identity_t<basic_string_view<CharT, Traits>> rhs
) noexcept
{
    return static_cast<Traits::comparison_category>(lhs.compare(rhs) <=> 0);
}

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_BASIC_STRING_VIEW_HPP_
