#ifndef ALKOS_KERNEL_INCLUDE_MEM_PAGE_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PAGE_HPP_

#include <extensions/bit.hpp>

#include "hal/constants.hpp"

struct Page {
    byte bytes[kPageSizeBytes];
};

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PAGE_HPP_
