#ifndef ALKOS_KERNEL_INCLUDE_INIT_HPP_
#define ALKOS_KERNEL_INCLUDE_INIT_HPP_

/**
 * @brief Performs all complex initialization, most of the code should be here,
 *        memory and kmalloc should be available to use at this stage.
 * @note  MUST ALWAYS be invoked after the PreKernelInit function
 */
void KernelInit();

#endif  // ALKOS_KERNEL_INCLUDE_INIT_HPP_
