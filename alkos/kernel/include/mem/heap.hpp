#ifndef ALKOS_KERNEL_INCLUDE_MEM_HEAP_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_HEAP_HPP_

#include <extensions/expected.hpp>

#include "mem/error.hpp"
#include "mem/types.hpp"

namespace Mem
{

// Heap
Expected<VPtr<void>, MemError> KMalloc(size_t size);
template <typename T>
Expected<VPtr<T>, MemError> KMalloc()
{
    return {};
    return KMalloc(sizeof(T)).transform([](void* ptr) {
        return reinterpret_cast<VPtr<T>>(ptr);
    });
}
template <typename T>
void KFree(VPtr<T> ptr);

template <>
void KFree(VPtr<void> ptr);

template <typename T>
void KFree(VPtr<T> ptr)
{
    KFree(reinterpret_cast<VPtr<void>>(ptr));
}

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_HEAP_HPP_
