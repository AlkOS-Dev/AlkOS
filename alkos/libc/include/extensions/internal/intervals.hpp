#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_INTERVALS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_INTERVALS_HPP_

#include "extensions/type_traits.hpp"
#include "extensions/types.hpp"

namespace internal
{

// TODO : Make Tests

template <typename NumT>
    requires std::is_arithmetic_v<NumT>
inline bool DoIntervalsOverlap(NumT start1, NumT end1, NumT start2, NumT end2)
{
    return start1 <= end2 && end1 >= start2;
}

template <typename NumT>
    requires std::is_arithmetic_v<NumT>
inline bool DoOpenIntervalsOverlap(NumT start1, NumT end1, NumT start2, NumT end2)
{
    return start1 < end2 && end1 > start2;
}

}  // namespace internal

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_INTERVALS_HPP_
