#ifndef ALKOS_KERNEL_ABI_SPINLOCK_HPP_
#define ALKOS_KERNEL_ABI_SPINLOCK_HPP_

#include <extensions/type_traits.hpp>

namespace arch
{
/* Should be defined by architecture */
class Spinlock;

/* Defined by architecture to allow various optimizations */
struct SpinlockAbi {
    /* basic operations */
    void Lock();
    void Unlock();
    bool TryLock();
    NODISCARD bool IsLocked() const;
};

}  // namespace arch

/* Load architecture definition of component */
#include <abi/spinlock.hpp>
static_assert(
    std::is_base_of_v<arch::SpinlockAbi, arch::Spinlock>,
    "Spinlock implementation must derive from the ABI"
);

#endif  // ALKOS_KERNEL_ABI_SPINLOCK_HPP_
