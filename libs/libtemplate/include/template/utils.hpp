#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_UTILS_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_UTILS_HPP_

#include <defines.hpp>
#include <type_traits.hpp>

namespace template_lib
{
// ------------------------------
// OPTIONAL_FIELD
// ------------------------------

// Apparently using OptionalField = std::conditional_t<...> does not work
#define OPTIONAL_FIELD(cond, type) NO_UNIQUE_ADDRESS std::conditional_t<cond, type, UNIQUE_EMPTY>

}  // namespace template_lib
#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_UTILS_HPP_
