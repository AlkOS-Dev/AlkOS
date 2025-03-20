#ifndef ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_INTERVALS_HPP_
#define ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_INTERVALS_HPP_

#include <extensions/types.hpp>

namespace internal
{

// TODO : Template
// TODO : Make Tests
inline bool DoIntervalsOverlap(i64 start1, i64 end1, i64 start2, i64 end2)
{
    return start1 <= end2 && end1 >= start2;
}

inline bool DoOpenIntervalsOverlap(i64 start1, i64 end1, i64 start2, i64 end2)
{
    return start1 < end2 && end1 > start2;
}

}  // namespace internal

#endif  // ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_INTERVALS_HPP_
