#ifndef KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_
#define KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_

#include "mem/phys/mngr/buddy.hpp"

namespace Mem
{

static constexpr bool kOnSlabFreelist  = true;
static constexpr bool kOffSlabFreelist = false;

template <class T, bool OnSlabFreeList = true>
class Slab
{
    // In constructor
    // Calc size of stored object
    // If size(T) > 512 -> OffSlabFreelist = true
    //
    // | First init Cache-Cache
    // | Slab<OffSlabFreelist = false>() sizes from 8 to 512
    // | OnSlabFreeList -> Self Contained Class (Except Buddy dependency)
    // | Allocates normal page and manages it -> Marks it as used by slab
    // | Alloc -> Gives item from freelist
    // | Free  -> Returns item to freelist
    //
    // If size(T) > 512:
    // Use Cache-Cache to init OffSlabFreeList
    // Rest is same
    //
    // Then make Fasade to create Slabs in simple manner
};

template <class T>
class Slab<T, kOnSlabFreelist>
{
    Slab(BuddyPmm b_pmm);
};

template <class T>
class Slab<T, kOffSlabFreelist>
{
};

}  // namespace Mem

#endif /* KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_ */
