#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_PATHS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_PATHS_HPP_

#include <extensions/array.hpp>
#include <extensions/string.hpp>
#include <extensions/tuple.hpp>

namespace path
{

static constexpr char kSeparator = '/';

namespace internal
{

template <auto Data>
inline constexpr const auto& MakeStatic = Data;

consteval auto TrimArray(auto callback)
{
    constexpr auto result_pair = callback();
    constexpr auto arr         = std::get<0>(result_pair);
    constexpr auto size        = std::get<1>(result_pair);
    std::array<char, size> result;
    for (size_t i = 0; i < size; ++i) result[i] = arr[i];
    return result;
}

template <typename Callback>
    requires requires(Callback callback) {
        typename std::remove_cvref_t<decltype(std::get<0>(callback()))>;
        { std::get<1>(callback()) } -> std::convertible_to<size_t>;
    }
consteval auto ConvertToStringView(Callback callback)
{
    constexpr auto& static_data = MakeStatic<TrimArray(callback)>;
    std::string_view result(static_data.data(), static_data.size() - 1);
    return result;
}

}  // namespace internal

consteval bool IsAbsolute(std::string_view path)
{
    return path.size() > 0 && path[0] == kSeparator;
}

template <size_t N>
consteval auto weakly_canonical(const std::string_view path)
{
    const size_t len       = path.size();
    const bool kIsAbsolute = IsAbsolute(path);

    std::array<size_t, N> separator_indices{};
    std::array<char, N> buffer{};
    size_t separator_count = 0;
    size_t out_pos         = 0;
    size_t base_pos        = (kIsAbsolute ? 1 : 0);

    if (len == 0) {
        buffer[0] = '\0';
        return std::make_tuple(buffer, 1_size);
    }

    if (kIsAbsolute) {
        // Record the first separator (root)
        separator_indices[separator_count++] = 0;
        buffer[0]                            = kSeparator;
        out_pos                              = 1;
    }

    size_t i = out_pos;
    while (i < len) {
        // Skip multiple consecutive separators
        if (path[i] == kSeparator && path[i - 1] == kSeparator) {
            i++;
            continue;
        }

        // Find the next segment
        size_t segment_start = i;
        while (i < len && path[i] != kSeparator) {
            i++;
        }
        size_t segment_len = i - segment_start;

        // Handle "." and ".." segments
        std::string_view segment(path.data() + segment_start, segment_len);
        if (segment == ".") {
            // Skip "." segment
            i++;
            continue;
        } else if (segment == "..") {
            // Go up one directory
            if (separator_count > 1) {
                out_pos = separator_indices[--separator_count - 1] + 1;
            } else if (separator_count == 1) {
                separator_count = (kIsAbsolute ? 1 : 0);
                out_pos         = base_pos;
            } else if (!kIsAbsolute) {
                // Copy the ".." segment
                for (size_t j = 0; j < segment_len && out_pos < buffer.size(); j++) {
                    buffer[out_pos++] = path[segment_start + j];
                }
                if (out_pos < buffer.size()) {
                    buffer[out_pos++]                  = kSeparator;
                    separator_indices[separator_count] = out_pos - 1;
                }
                base_pos = out_pos;
            }
            i++;
            continue;
        }

        // Copy the segment to the output buffer
        for (size_t j = 0; j < segment_len && out_pos < buffer.size(); j++) {
            buffer[out_pos++] = path[segment_start + j];
        }

        // Add separator if not end of path
        if (out_pos < buffer.size() && i < len && path[i] == kSeparator) {
            buffer[out_pos++]                    = kSeparator;
            separator_indices[separator_count++] = out_pos - 1;
        }

        i++;
    }

    // If result is empty for relative path, use "."
    if (!kIsAbsolute && out_pos == 0) {
        buffer[0] = '.';
        out_pos   = 1;
    }

    // Remove trailing separator if result ends with ".."
    if (!kIsAbsolute && out_pos == base_pos && buffer[out_pos - 1] == kSeparator) {
        out_pos--;
    }

    // Null-terminate
    if (out_pos < buffer.size()) {
        buffer[out_pos++] = '\0';
    }

    return std::make_tuple(buffer, out_pos);
}

template <size_t MaxSize1, size_t MaxSize2>
consteval auto lexically_relative(const std::string_view path, const std::string_view base_path)
{
    constexpr size_t kMaxSize = 2 * (MaxSize1 + MaxSize2);

    const size_t path_len = path.size();
    const size_t base_len = base_path.size();

    // Check if both are absolute or both are relative
    bool path_absolute = IsAbsolute(path);
    bool base_absolute = IsAbsolute(base_path);

    if (path_absolute != base_absolute) {
        // Different path types, return "."
        std::array<char, kMaxSize> result{};
        result[0] = '\0';
        return std::make_tuple(result, 1_size);
    }

    // Parse path elements inline
    struct Element {
        size_t start;
        size_t length;
    };

    std::array<Element, kMaxSize> path_elements{};
    std::array<Element, kMaxSize> base_elements{};
    size_t path_count = 0;
    size_t base_count = 0;

    // Parse path elements
    size_t start = path_absolute ? 1 : 0;  // Skip root separator if absolute
    while (start < path_len && path_count < kMaxSize) {
        // Skip consecutive separators
        while (start < path_len && path[start] == kSeparator) {
            start++;
        }
        if (start >= path_len)
            break;

        // Find end of current element
        size_t end = start;
        while (end < path_len && path[end] != kSeparator) {
            end++;
        }

        if (end > start) {  // Non-empty element
            path_elements[path_count] = {start, end - start};
            path_count++;
        }
        start = end;
    }

    // Parse base elements
    start = base_absolute ? 1 : 0;  // Skip root separator if absolute
    while (start < base_len && base_count < kMaxSize) {
        // Skip consecutive separators
        while (start < base_len && base_path[start] == kSeparator) {
            start++;
        }
        if (start >= base_len)
            break;

        // Find end of current element
        size_t end = start;
        while (end < base_len && base_path[end] != kSeparator) {
            end++;
        }

        if (end > start) {  // Non-empty element
            base_elements[base_count] = {start, end - start};
            base_count++;
        }
        start = end;
    }

    // Find mismatch point - find first position where elements differ
    size_t mismatch_pos = 0;
    while (mismatch_pos < path_count && mismatch_pos < base_count) {
        const auto& path_elem = path_elements[mismatch_pos];
        const auto& base_elem = base_elements[mismatch_pos];

        if (path_elem.length != base_elem.length)
            break;

        bool elements_equal = true;
        for (size_t i = 0; i < path_elem.length; ++i) {
            if (path[path_elem.start + i] != base_path[base_elem.start + i]) {
                elements_equal = false;
                break;
            }
        }
        if (!elements_equal)
            break;
        mismatch_pos++;
    }

    // If all elements match and counts are equal, paths are equivalent
    if (mismatch_pos == path_count && mismatch_pos == base_count) {
        std::array<char, kMaxSize> result{};
        result[0] = '.';
        result[1] = '\0';
        return std::make_tuple(result, 2_size);
    }

    // N = number of nonempty filename elements that are neither dot nor dot-dot,
    // minus the number of dot-dot filename elements
    size_t regular_elements = 0;
    size_t dotdot_elements  = 0;

    // Count from mismatch point to end of base_path
    for (size_t i = mismatch_pos; i < base_count; ++i) {
        const auto& elem = base_elements[i];
        // Check if element is neither dot nor dot-dot
        bool is_dot = (elem.length == 1 && base_path[elem.start] == '.');
        bool is_dotdot =
            (elem.length == 2 && base_path[elem.start] == '.' && base_path[elem.start + 1] == '.');

        if (!is_dot && !is_dotdot) {
            regular_elements++;
        } else if (is_dotdot) {
            dotdot_elements++;
        }
    }

    if (dotdot_elements > regular_elements) {
        std::array<char, kMaxSize> result{};
        result[0] = '\0';
        return std::make_tuple(result, 1_size);
    }

    size_t N = regular_elements - dotdot_elements;
    std::array<char, kMaxSize> result{};
    size_t out_pos = 0;

    // Add N applications of ".." (one for each remaining regular element in base)
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) {
            result[out_pos++] = kSeparator;
        }
        result[out_pos++] = '.';
        result[out_pos++] = '.';
    }

    // Add remaining elements from path after the common prefix
    for (size_t i = mismatch_pos; i < path_count && out_pos < kMaxSize; ++i) {
        if (out_pos > 0) {
            result[out_pos++] = kSeparator;
        }
        const auto& element = path_elements[i];
        for (size_t j = 0; j < element.length && out_pos < kMaxSize; ++j) {
            result[out_pos++] = path[element.start + j];
        }
    }

    // If result is empty, return "."
    if (out_pos == 0) {
        result[0] = '.';
        out_pos   = 1;
    }

    // Null-terminate
    if (out_pos < kMaxSize) {
        result[out_pos++] = '\0';
    }

    return std::make_tuple(result, out_pos);
}

}  // namespace path

#define TRANSFORM_STRING(transform, str, ...)                          \
    path::internal::ConvertToStringView([]() consteval {               \
        return transform<sizeof(str)>(str __VA_OPT__(, ) __VA_ARGS__); \
    })

#define RELATIVE(base_path, target_path)                                                       \
    path::internal::ConvertToStringView([]() consteval {                                       \
        constexpr auto path_canonical = TRANSFORM_STRING(path::weakly_canonical, target_path); \
        constexpr auto base_canonical = TRANSFORM_STRING(path::weakly_canonical, base_path);   \
        constexpr auto path_size      = path_canonical.size();                                 \
        constexpr auto base_size      = base_canonical.size();                                 \
        return path::lexically_relative<path_size, base_size>(path_canonical, base_canonical); \
    })

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_PATHS_HPP_
