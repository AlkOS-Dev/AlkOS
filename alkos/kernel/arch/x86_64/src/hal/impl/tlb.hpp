#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TLB_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TLB_HPP_

#include <hal/api/tlb.hpp>

#include <mem/virt/addr_space.hpp>
#include "cpu/control_registers.hpp"

namespace arch
{

class Tlb : public TlbAPI
{
    public:
    void FlushAll();
    void InvalidatePage(Mem::VPtr<void> vaddr);
    void SwitchAddrSpace(Mem::VPtr<Mem::AddressSpace> as);

    private:
};

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TLB_HPP_
