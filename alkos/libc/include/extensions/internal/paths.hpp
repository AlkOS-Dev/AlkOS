#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_PATHS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_PATHS_HPP_

#include <extensions/array.hpp>
#include <extensions/string.hpp>
#include <extensions/tuple.hpp>

namespace internal
{

template <auto Data>
inline constexpr const auto &MakeStatic = Data;

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
    constexpr auto &static_data = MakeStatic<TrimArray(callback)>;
    std::string_view result(static_data.data(), static_data.size());
    return result;
}

}  // namespace internal

template <size_t N>
consteval auto weakly_canonical_path(const char (&absolute_path)[N])
{
    constexpr char kPathSeparator = '/';
    constexpr size_t len          = N - 1;

    std::array<size_t, N> separator_indices{};
    std::array<char, N> buffer{};
    size_t separator_count = 0;

    if (len == 0 || absolute_path[0] != kPathSeparator) {
        // Return empty if invalid path
        return std::make_tuple(buffer, 1_size);
    }

    // Record the first separator (root)
    separator_indices[separator_count++] = 0;
    buffer[0]                            = kPathSeparator;
    size_t out_pos                       = 1;

    size_t i = 1;
    while (i < len) {
        // Skip multiple consecutive separators
        if (absolute_path[i] == kPathSeparator && absolute_path[i - 1] == kPathSeparator) {
            i++;
            continue;
        }

        // Find the next segment
        size_t segment_start = i;
        while (i < len && absolute_path[i] != kPathSeparator) {
            i++;
        }
        size_t segment_len = i - segment_start;

        // Handle "." and ".." segments
        std::string_view segment(absolute_path + segment_start, segment_len);
        if (segment == ".") {
            // Skip "." segment
            i++;
            continue;
        } else if (segment == "..") {
            // Go up one directory
            if (separator_count > 1) {
                out_pos = separator_indices[--separator_count - 1] + 1;
            }
            i++;
            continue;
        }

        // Copy the segment to the output buffer
        for (size_t j = 0; j < segment_len && out_pos < N - 1; j++) {
            buffer[out_pos++] = absolute_path[segment_start + j];
        }

        // Add separator if not end of path
        if (i < len && out_pos < N - 1) {
            buffer[out_pos++]                    = kPathSeparator;
            separator_indices[separator_count++] = out_pos - 1;
        }

        i++;
    }

    // Add null terminator
    if (out_pos < N) {
        buffer[out_pos] = '\0';
    }

    return std::make_tuple(buffer, out_pos + 1);
}

#define TRANSFORM_STRING(transform, str)           \
    internal::ConvertToStringView([]() consteval { \
        return transform(str);                     \
    })

#define RELATIVE(path) TRANSFORM_STRING(weakly_canonical_path, path)

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_PATHS_HPP_
