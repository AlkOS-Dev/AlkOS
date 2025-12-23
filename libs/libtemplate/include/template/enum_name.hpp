#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_ENUM_NAME_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_ENUM_NAME_HPP_

#include <string.hpp>
#include <utility.hpp>

namespace template_lib
{
template <auto V>
constexpr std::string_view pretty_name()
{
    std::string_view name = __PRETTY_FUNCTION__;

    size_t begin = name.find("V = ");
    if (begin == std::string_view::npos)
        return "Unknown";
    begin += 4;

    size_t end = name.find_first_of("];", begin);
    if (end == std::string_view::npos)
        end = name.length();

    std::string_view value_scope = name.substr(begin, end - begin);

    size_t last_colon = value_scope.rfind("::");
    if (last_colon != std::string_view::npos) {
        return value_scope.substr(last_colon + 2);
    }

    return value_scope;
}

template <typename E, int... Is>
constexpr std::string_view convert_impl(E value, std::integer_sequence<int, Is...>)
{
    std::string_view res = "Unknown";
    ((static_cast<int>(value) == Is ? (res = pretty_name<static_cast<E>(Is)>(), 0) : 0), ...);
    return res;
}

template <typename E>
constexpr std::string_view to_string(E value)
{
    return convert_impl(value, std::make_integer_sequence<int, 50>{});
}
}  // namespace template_lib

#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_ENUM_NAME_HPP_
