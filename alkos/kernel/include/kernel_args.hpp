#ifndef ALKOS_KERNEL_INCLUDE_KERNEL_ARGS_HPP_
#define ALKOS_KERNEL_INCLUDE_KERNEL_ARGS_HPP_

#include "hal/kernel.hpp"
#include "mem/phys/ptr.hpp"
#include "mem/virt/ptr.hpp"

struct KernelArguments {
    mem::VirtualPtr<void> kernel_start;
    mem::VirtualPtr<void> kernel_end;
    mem::PhysicalPtr<void> root_page_table;
    mem::PhysicalPtr<void> mem_bitmap;
    size_t total_page_frames;
    mem::PhysicalPtr<void> multiboot_info;
};

KernelArguments SanitizeKernelArgs(const hal::RawKernelArguments raw_args);

#endif  // ALKOS_KERNEL_INCLUDE_KERNEL_ARGS_HPP_
