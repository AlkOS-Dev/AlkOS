#ifndef LIBC_INCLUDE_EXTENSIONS_CSTDDEF_HPP_
#define LIBC_INCLUDE_EXTENSIONS_CSTDDEF_HPP_

#include <stddef.h>

namespace std
{
using nullptr_t   = ::nullptr_t;
using size_t      = ::size_t;
using ptrdiff_t   = ::ptrdiff_t;
using max_align_t = ::max_align_t;
}  // namespace std

#endif  // LIBC_INCLUDE_EXTENSIONS_CSTDDEF_HPP_
