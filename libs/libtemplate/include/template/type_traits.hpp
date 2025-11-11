#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_TYPE_TRAITS_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_TYPE_TRAITS_HPP_

#include <limits>
#include <types.hpp>

namespace template_lib
{

template <size_t V>
struct minimal_unsigned_type {
    using type = decltype([] {
        if constexpr (V <= std::numeric_limits<u8>::max()) {
            return u8{};
        } else if constexpr (V <= std::numeric_limits<u16>::max()) {
            return u16{};
        } else if constexpr (V <= std::numeric_limits<u32>::max()) {
            return u32{};
        } else {
            return u64{};
        }
    }());
};

template <size_t V>
using minimal_unsigned_type_t = typename minimal_unsigned_type<V>::type;

}  // namespace template_lib
#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_TYPE_TRAITS_HPP_
