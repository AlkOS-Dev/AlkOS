#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_INTEGRAL_STORAGE_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_INTEGRAL_STORAGE_HPP_

#include "bits_ext.hpp"

namespace template_lib
{
template <size_t kNum>
using MinimalUnsignedStorage_t = UnsignedIntegral_t<
    std::bit_width(kNum) / 8 == 0 ? 1 : std::bit_ceil(static_cast<u32>(std::bit_width(kNum)) / 8)>;

}  // namespace template_lib

#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_INTEGRAL_STORAGE_HPP_
