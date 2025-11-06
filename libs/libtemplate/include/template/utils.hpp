#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_UTILS_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_UTILS_HPP_

#include <variant.hpp>

namespace template_lib
{
// ------------------------------
// OptionalField
// ------------------------------

template <bool cond, class T>
using OptionalField = std::conditional_t<cond, T, std::monostate>;
}  // namespace template_lib
#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_UTILS_HPP_
