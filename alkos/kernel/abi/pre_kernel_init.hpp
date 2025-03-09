#ifndef KERNEL_ABI_INIT_HPP_
#define KERNEL_ABI_INIT_HPP_

#include <loader_data.hpp>
#include "extensions/types.hpp"

/**
 * @brief Performs all necessary hardware initialization on the system.
 * @note This function should set up system-critical components like memory, I/O devices, and
 *       interrupt controllers. It must be called before any kernel functionality is used,
 *       and before the GCC compiler's _init function.
 * @note Global constructors will not be called inside this function, as it must execute
 *       before the _init function is invoked.
 */
extern "C" void PreKernelInit(LoaderData* loader_data);

#endif  // KERNEL_ABI_INIT_HPP_
