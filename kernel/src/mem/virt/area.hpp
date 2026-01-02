#ifndef KERNEL_SRC_MEM_VIRT_AREA_HPP_
#define KERNEL_SRC_MEM_VIRT_AREA_HPP_

#include <types.h>
#include "mem/types.hpp"
#include "mem/virt/page_fault_data.hpp"

namespace Mem
{

class AddressSpace;

struct VirtualMemAreaFlags {
    bool readable : 1;
    bool writable : 1;
    bool executable : 1;
    bool write_through : 1 = false;
    bool cache_disable : 1 = false;
};
using VMemAreaFlags = VirtualMemAreaFlags;

/**
 * @brief Abstract base class representing a Virtual Memory Area.
 * Defines the range, permissions, and behavior on page faults.
 */
class VMemArea
{
    public:
    VMemArea(VPtr<void> start, size_t size, VirtualMemAreaFlags flags)
        : start_(start), size_(size), flags_(flags)
    {
    }

    virtual ~VMemArea() = default;

    /**
     * @brief Handles a page fault occurring within this area.
     * @param fault_addr The exact virtual address that caused the fault.
     * @param err The error code provided by the CPU.
     * @param as The address space this area belongs to.
     * @return true if the fault was resolved, false otherwise.
     */
    virtual bool HandleFault(
        VPtr<void> fault_addr, const PageFaultData::ErrorCode &err, AddressSpace &as
    ) = 0;

    // Getters
    NODISCARD VPtr<void> GetStart() const { return start_; }
    NODISCARD size_t GetSize() const { return size_; }
    NODISCARD VPtr<void> GetEnd() const
    {
        return reinterpret_cast<VPtr<void>>(reinterpret_cast<uptr>(start_) + size_);
    }
    NODISCARD VirtualMemAreaFlags GetFlags() const { return flags_; }

    // Mutators
    void SetFlags(VirtualMemAreaFlags flags) { flags_ = flags; }

    protected:
    VPtr<void> start_;
    size_t size_;
    VirtualMemAreaFlags flags_;
};

/**
 * @brief Represents anonymous memory (RAM), zero-initialized on demand.
 */
class AnonymousVMemArea final : public VMemArea
{
    public:
    using VMemArea::VMemArea;

    bool HandleFault(
        VPtr<void> fault_addr, const PageFaultData::ErrorCode &err, AddressSpace &as
    ) override;
};

/**
 * @brief Represents a direct mapping to a physical address range (e.g., MMIO).
 */
class DirectMappingVMemArea final : public VMemArea
{
    public:
    DirectMappingVMemArea(
        VPtr<void> start, size_t size, VirtualMemAreaFlags flags, PPtr<void> phys_start
    )
        : VMemArea(start, size, flags), phys_start_(phys_start)
    {
    }

    bool HandleFault(
        VPtr<void> fault_addr, const PageFaultData::ErrorCode &err, AddressSpace &as
    ) override;

    private:
    PPtr<void> phys_start_;
};

/**
 * @brief Represents the kernel address space area.
 * Handles lazy synchronization of kernel mappings into user address spaces.
 */
class KernelSyncVMemArea final : public VMemArea
{
    public:
    KernelSyncVMemArea();

    bool HandleFault(
        VPtr<void> fault_addr, const PageFaultData::ErrorCode &err, AddressSpace &as
    ) override;
};

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_VIRT_AREA_HPP_
