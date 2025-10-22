#ifndef ALKOS_KERNEL_INCLUDE_MEM_HEAP_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_HEAP_HPP_

#include <extensions/expected.hpp>

#include "mem/error.hpp"
#include "mem/types.hpp"

namespace Mem
{

struct KMallocRequest {
    size_t size      = 0;
    size_t alignment = 0;
    // TODO: zones
};

// Heap
Expected<VPtr<void>, MemError> KMalloc(KMallocRequest request);
template <typename T>
Expected<VPtr<T>, MemError> KMalloc(KMallocRequest request)
{
    request.size = sizeof(T);
    return KMalloc(request).transform([](void *ptr) {
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
