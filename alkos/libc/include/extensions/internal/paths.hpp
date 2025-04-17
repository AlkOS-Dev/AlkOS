#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_PATHS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_PATHS_HPP_

#include "string.h"

namespace internal
{

// #include <array>
// #include <string_view>
// #include <cstddef>
// #include <stdio.h>
// #include <concepts>
// #include <functional>
// #include <string>
// #include <type_traits>
// #include <stdint.h>

template <auto Data>
inline constexpr const auto &MakeStatic = Data;

struct __ArrayHelper {
    static constexpr size_t kMaxStringLength = INT32_MAX;
    std::array<char, kMaxStringLength> data{};
    size_t size;

    constexpr char &operator[](std::size_t i) { return data[i]; }
    constexpr char const &operator[](std::size_t i) const { return data[i]; }

    constexpr __ArrayHelper(std::string str)
    {
        size = str.length() + 1;
        for (size_t i = 0; i < size; ++i) data[i] = str[i];
    }
};

consteval auto __TrimArray(auto callback)
{
    constexpr auto arr = __ArrayHelper(callback());
    std::array<char, arr.size> result;
    for (size_t i = 0; i < arr.size; ++i) result[i] = arr[i];
    return result;
}

template <typename F>
concept StringCallback = requires(F f) {
    { f() } -> std::convertible_to<std::string>;
};

template <StringCallback Callback>
consteval auto __ConvertToString(Callback callback)
{
    constexpr auto &static_data = MakeStatic<__TrimArray(callback)>;
    std::basic_string_view<char> result(static_data.begin(), static_data.end());
    return result.data();
}

#define TRANSFORM_STRING(transform, string, ...)             \
    __ConvertToString([]() {                                 \
        return transform(string __VA_OPT__(, ) __VA_ARGS__); \
    })

constexpr std::string weakly_canonical_path(std::string_view absolute_path)
{
    constexpr size_t kMaxPathLength = 256;
    constexpr char kPathSeparator   = '/';
    size_t separator_indices[kMaxPathLength];
    std::array<char, kMaxPathLength> buffer;
    size_t separator_count = 0;

    const size_t len = absolute_path.length();

    if (*absolute_path.data() != kPathSeparator || len >= kMaxPathLength) {
        return std::string(absolute_path);  // Path too long or not absolute
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
        if (segment_len == 1 && absolute_path[segment_start] == '.') {
            // Skip "." segment
            i++;
            continue;
        } else if (segment_len == 2 && absolute_path[segment_start] == '.' &&
                   absolute_path[segment_start + 1] == '.') {
            // Handle ".." segment by going up one level
            if (separator_count > 1) {
                out_pos = separator_indices[--separator_count - 1] + 1;
            }
            i++;
            continue;
        }

        // Copy the segment to the output buffer
        for (size_t j = 0; j < segment_len; j++) {
            buffer[out_pos++] = absolute_path[segment_start + j];
        }

        // Add separator if not end of path
        if (i < len) {
            buffer[out_pos++]                    = kPathSeparator;
            separator_indices[separator_count++] = out_pos - 1;
        }

        i++;
    }

    // Add null terminator
    buffer[out_pos] = '\0';

    return std::string(buffer.begin(), buffer.begin() + out_pos);
}

}  // namespace internal

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_PATHS_HPP_
