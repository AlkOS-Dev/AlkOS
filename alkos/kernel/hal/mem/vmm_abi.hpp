#ifndef ALKOS_KERNEL_ABI_VMM_ABI_HPP_
#define ALKOS_KERNEL_ABI_VMM_ABI_HPP_

#include <extensions/expected.hpp>
#include <extensions/type_traits.hpp>
#include "mem/error.hpp"
#include "mem/phys_ptr.hpp"

namespace arch
{

/* Should be defined by architecture */
class VirtualMemoryManager;

// TODO: Add phys page struct

/* Arch implementation should follow this ABI */
struct VirtualMemoryManagerABI {
};

}  // namespace arch

#endif  // ALKOS_KERNEL_ABI_VMM_ABI_HPP_
