#ifndef KERNEL_SRC_VFS_PATH_HPP_
#define KERNEL_SRC_VFS_PATH_HPP_

#include <array.hpp>
#include <concepts.hpp>
#include <string.hpp>
#include <type_traits.hpp>

namespace vfs
{

// Constants
inline constexpr size_t kMaxPathSize      = 1024;
inline constexpr size_t kMaxComponents    = 64;
inline constexpr size_t kMaxComponentSize = 128;
inline constexpr char kPathSeparator      = '/';

// Ccncepts
template <typename T>
concept PathStringLike = std::convertible_to<T, std::string_view>;

template <typename Callback>
concept PathComponentCallback = requires(Callback cb, std::string_view sv) {
    { cb(sv) } -> std::same_as<void>;
};

// Path component const iterator
class PathIterator
{
    public:
    using value_type      = std::string_view;
    using difference_type = std::ptrdiff_t;
    using pointer         = const std::string_view *;
    using reference       = const std::string_view &;

    PathIterator() = delete;
    PathIterator(const std::array<std::string_view, kMaxComponents> &components, size_t index)
        : components_(components), index_(index)
    {
    }

    reference operator*() const { return components_[index_]; }
    pointer operator->() const { return &components_[index_]; }

    PathIterator &operator++()
    {
        ++index_;
        return *this;
    }
    PathIterator operator++(int)
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const PathIterator &other) const
    {
        return components_ == other.components_ && index_ == other.index_;
    }

    private:
    const std::array<std::string_view, kMaxComponents> &components_;
    size_t index_ = 0;
};

class Path
{
    public:
    static const Path kCurrentDir;
    static const Path kParentDir;
    static const Path kRoot;

    Path() = default;

    template <PathStringLike T>
    explicit Path(T &&path_str)
    {
        std::string_view sv(std::forward<T>(path_str));
        path_len_ = std::min(sv.length(), kMaxPathSize - 1);
        strncpy(raw_path_.data(), sv.data(), path_len_);
        raw_path_[path_len_] = '\0';
        Parse();
    }

    Path(const Path &other)
    {
        raw_path_ = other.raw_path_;
        path_len_ = other.path_len_;
        Parse();
    }

    Path &operator=(const Path &other)
    {
        if (this != &other) {
            raw_path_ = other.raw_path_;
            path_len_ = other.path_len_;
            Parse();
        }
        return *this;
    }

    Path(Path &&other) noexcept
    {
        raw_path_ = other.raw_path_;
        path_len_ = other.path_len_;
        Parse();
    }

    Path &operator=(Path &&other) noexcept
    {
        if (this != &other) {
            raw_path_ = other.raw_path_;
            path_len_ = other.path_len_;
            Parse();
        }
        return *this;
    }

    // Basic properties
    bool IsEmpty() const noexcept { return path_len_ == 0; }
    bool IsAbsolute() const noexcept { return is_absolute_; }
    bool IsRelative() const noexcept { return !is_absolute_; }
    bool IsRoot() const noexcept { return is_absolute_ && num_components_ == 0; }

    const char *CString() const noexcept { return raw_path_.data(); }
    std::string_view StringView() const noexcept
    {
        return std::string_view(raw_path_.data(), path_len_);
    }

    // Component access
    size_t ComponentCount() const noexcept { return num_components_; }
    bool HasComponents() const noexcept { return num_components_ > 0; }

    std::string_view operator[](size_t index) const noexcept
    {
        return index < num_components_ ? components_[index] : std::string_view{};
    }

    std::string_view GetComponent(size_t index) const noexcept { return (*this)[index]; }

    std::string_view GetFilename() const noexcept
    {
        return num_components_ == 0 ? std::string_view{} : components_[num_components_ - 1];
    }

    std::string_view GetStem() const noexcept
    {
        auto filename = GetFilename();
        if (filename.empty())
            return {};
        auto dot_pos = filename.find_last_of('.');
        return (dot_pos == std::string_view::npos || dot_pos == 0) ? filename
                                                                   : filename.substr(0, dot_pos);
    }

    std::string_view GetExtension() const noexcept
    {
        auto filename = GetFilename();
        if (filename.empty())
            return {};
        auto dot_pos = filename.find_last_of('.');
        return (dot_pos == std::string_view::npos || dot_pos == 0) ? std::string_view{}
                                                                   : filename.substr(dot_pos);
    }

    Path GetParent() const
    {
        if (IsRoot() || num_components_ == 0) {
            return IsAbsolute() ? kRoot : kCurrentDir;
        }

        // If this is a relative path with only one component, parent is current dir
        if (IsRelative() && num_components_ == 1) {
            return kCurrentDir;
        }

        Path parent;
        parent.is_absolute_    = is_absolute_;
        parent.num_components_ = num_components_ - 1;
        strncpy(parent.raw_path_.data(), raw_path_.data(), std::min(path_len_, kMaxPathSize - 1));
        for (size_t i = 0; i < parent.num_components_; ++i) {
            parent.components_[i] = components_[i];
        }
        parent.RebuildPath();
        return parent;
    }

    // Iterator interface
    using iterator       = PathIterator;
    using const_iterator = PathIterator;

    iterator begin() { return PathIterator(components_, 0); }
    iterator end() { return PathIterator(components_, num_components_); }
    const_iterator begin() const { return PathIterator(components_, 0); }
    const_iterator end() const { return PathIterator(components_, num_components_); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    // Path manipulation
    Path operator/(const Path &other) const
    {
        if (other.IsAbsolute())
            return other;

        Path result = *this;
        // Check for component overflow before appending
        if (result.num_components_ + other.num_components_ <= kMaxComponents) {
            for (size_t i = 0; i < other.num_components_; ++i) {
                result.components_[result.num_components_ + i] = other.components_[i];
            }
            result.num_components_ += other.num_components_;
            result.RebuildPath();
        }
        return result;
    }

    template <PathStringLike T>
    Path operator/(T &&component) const
    {
        return *this / Path(std::forward<T>(component));
    }

    Path &operator/=(const Path &other)
    {
        *this = *this / other;
        return *this;
    }

    template <PathStringLike T>
    Path &operator/=(T &&component)
    {
        return *this /= Path(std::forward<T>(component));
    }

    // Canonical path operations
    Path GetWeaklyCanonical() const
    {
        Path canonical;
        canonical.is_absolute_ = is_absolute_;

        for (size_t i = 0; i < num_components_; ++i) {
            const auto &component = components_[i];

            if (component == kCurrentDir.StringView()) {
                continue;  // Skip "."
            }

            if (component == kParentDir.StringView()) {
                if (canonical.num_components_ > 0 &&
                    canonical.components_[canonical.num_components_ - 1] !=
                        kParentDir.StringView()) {
                    canonical.num_components_--;  // Pop back
                } else if (canonical.IsRelative()) {
                    if (canonical.num_components_ < kMaxComponents) {
                        canonical.components_[canonical.num_components_++] = component;
                    }
                }
            } else {
                if (canonical.num_components_ < kMaxComponents) {
                    canonical.components_[canonical.num_components_++] = component;
                }
            }
        }

        canonical.RebuildPath();
        return canonical;
    }

    Path GetNormalized() const { return GetWeaklyCanonical(); }

    // Callback-based iteration
    template <PathComponentCallback Callback>
    void ForEachComponent(Callback &&callback) const
    {
        for (size_t i = 0; i < num_components_; ++i) {
            if constexpr (std::same_as<std::invoke_result_t<Callback, std::string_view>, bool>) {
                if (!callback(components_[i]))
                    break;
            } else {
                callback(components_[i]);
            }
        }
    }

    template <PathComponentCallback Callback>
    void ForEachComponentReverse(Callback &&callback) const
    {
        if (num_components_ == 0)
            return;
        for (size_t i = num_components_; i-- > 0;) {
            if constexpr (std::same_as<std::invoke_result_t<Callback, std::string_view>, bool>) {
                if (!callback(components_[i]))
                    break;
            } else {
                callback(components_[i]);
            }
        }
    }

    // Comparison operators
    bool operator==(const Path &other) const noexcept
    {
        if (is_absolute_ != other.is_absolute_ || num_components_ != other.num_components_) {
            return false;
        }

        for (size_t i = 0; i < num_components_; ++i) {
            if (components_[i] != other.components_[i]) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const Path &other) const noexcept { return !(*this == other); }

    bool operator<(const Path &other) const noexcept
    {
        if (is_absolute_ != other.is_absolute_) {
            return !is_absolute_;  // Relative paths are less than absolute
        }

        size_t min_components = std::min(num_components_, other.num_components_);
        for (size_t i = 0; i < min_components; ++i) {
            if (components_[i] < other.components_[i]) {
                return true;
            } else if (components_[i] > other.components_[i]) {
                return false;
            }
        }
        // If all common components are equal, shorter path is less
        return num_components_ < other.num_components_;
    }

    // Static utility methods

    template <PathStringLike... Args>
    static Path Join(Args &&...args)
    {
        Path result;
        ((result /= Path(std::forward<Args>(args))), ...);
        return result;
    }

    private:
    void Parse()
    {
        num_components_ = 0;
        std::string_view full_path(raw_path_.data(), path_len_);

        if (full_path.empty()) {
            is_absolute_ = false;
            return;
        }

        is_absolute_ = (full_path[0] == kPathSeparator);

        size_t start = is_absolute_ ? 1 : 0;
        size_t pos   = start;

        while (pos < full_path.size() && num_components_ < kMaxComponents) {
            size_t next_sep = full_path.find(kPathSeparator, pos);
            if (next_sep == std::string_view::npos) {
                next_sep = full_path.size();
            }

            if (next_sep > pos) {  // Non-empty component
                components_[num_components_++] = full_path.substr(pos, next_sep - pos);
            }

            pos = next_sep + 1;
        }
    }

    void RebuildPath()
    {
        char *current         = raw_path_.data();
        const char *const end = raw_path_.data() + kMaxPathSize;

        if (is_absolute_) {
            if (current < end) {
                *current++ = kPathSeparator;
            }
        }

        for (size_t i = 0; i < num_components_; ++i) {
            if (i > 0) {
                if (current < end) {
                    *current++ = kPathSeparator;
                }
            }
            // Ensure we don't write past the end of the buffer
            size_t copy_len = std::min(components_[i].length(), static_cast<size_t>(end - current));
            strncpy(current, components_[i].data(), copy_len);
            current += copy_len;
        }

        path_len_ = current - raw_path_.data();
        if (path_len_ < kMaxPathSize) {
            raw_path_[path_len_] = '\0';
        }

        // Re-point all string_views to our internal buffer
        is_absolute_          = (path_len_ > 0 && raw_path_[0] == kPathSeparator);
        size_t current_offset = is_absolute_ ? 1 : 0;
        for (size_t i = 0; i < num_components_; ++i) {
            size_t component_len = components_[i].length();
            components_[i] = std::string_view(raw_path_.data() + current_offset, component_len);
            current_offset += component_len + 1;  // +1 for the separator
        }
    }

    // Member variables
    std::array<char, kMaxPathSize> raw_path_{};
    size_t path_len_ = 0;

    std::array<std::string_view, kMaxComponents> components_{};
    size_t num_components_ = 0;

    bool is_absolute_ = false;
};

inline const Path Path::kCurrentDir = Path(".");
inline const Path Path::kParentDir  = Path("..");
inline const Path Path::kRoot       = Path(std::string_view(&kPathSeparator, 1));

}  // namespace vfs

#endif  // KERNEL_SRC_VFS_PATH_HPP_
