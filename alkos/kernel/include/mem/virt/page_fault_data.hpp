#ifndef ALKOS_KERNEL_INCLUDE_MEM_VIRT_PAGE_FAULT_DATA_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VIRT_PAGE_FAULT_DATA_HPP_

#include "mem/types.hpp"

namespace Mem
{

/// Carries data used in generic page fault
struct PageFaultData {
    VPtr<void> faulting_ptr;
    struct ErrorCode {
        /// If true, the fault was a page-protection violation.
        /// If false, the fault was because the page was not present.
        bool present : 1;

        /// If true, the access that caused the fault was a write.
        /// If false, it was a read.
        bool write : 1;

        /// If true, the fault occurred while in user mode.
        /// If false, it occurred in supervisor (kernel) mode.
        bool user : 1;

        /// If true, one or more reserved bits were set in a page-table entry.
        bool reserved_bits : 1;

        /// If true, the fault was caused by an instruction fetch.
        bool instruction_fetch : 1;
    } error;
};

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VIRT_PAGE_FAULT_DATA_HPP_
