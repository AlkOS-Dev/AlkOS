#ifndef ALKOS_KERNEL_ABI_PMM_HPP_
#define ALKOS_KERNEL_ABI_PMM_HPP_

#include <extensions/expected.hpp>
#include <extensions/type_traits.hpp>
#include "mem/error.hpp"
#include "mem/phys_ptr.hpp"

namespace arch
{

/* Should be defined by architecture */
class PhysicalMemoryManager;

struct Page {
    byte bytes[kPageSizeBytes];
};

enum class AllocFlags : u64 { kNone = 0, kZeroed = 1 << 0 };

struct AllocationRequest {
    u64 num_pages    = 1;  // > 1 implies Contiguity
    u64 alignment    = kPageSizeBytes;
    AllocFlags flags = AllocFlags::kNone;

    PhysicalPtr<void> min_address = PhysicalPtr<void>(nullptr);
    PhysicalPtr<void> max_address = PhysicalPtr<void>(reinterpret_cast<void *>(~0ULL));
};

/* Arch implementation should follow this ABI */
struct PhysicalMemoryManagerABI {
    std::expected<PhysicalPtr<Page>, MemError> Alloc(AllocationRequest req = {});
    void Free(PhysicalPtr<Page> page);
};

}  // namespace arch

/* Load architecture definition of component */
#include <hal/mem/pmm.hpp>

#endif  // ALKOS_KERNEL_ABI_PMM_HPP_
