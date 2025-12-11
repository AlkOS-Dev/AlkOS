#ifndef LIBS_LIBCONTAINERS_INCLUDE_ALLOCATORS_CUSTOM_ALLOCATOR_WRAPPER_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_ALLOCATORS_CUSTOM_ALLOCATOR_WRAPPER_HPP_

#include <mem/cyclic_allocator.hpp>
#include <mem/heap.hpp>
#include <template_lib.hpp>
#include <type_traits.hpp>

namespace allocators
{

template <decltype(auto) AllocateFunc, decltype(auto) DeallocateFunc, typename AllocatorT = void>
class CustomAllocatorWrapper : decltype(AllocateFunc), decltype(DeallocateFunc)
{
    static constexpr bool kCondition = !std::is_void_v<AllocatorT>;

    public:
    template <typename U, typename... Args>
    FORCE_INLINE_F U *Allocate(Args &&...args)
    {
        return decltype(AllocateFunc)::operator()(std::forward<Args>(args)...);
    }

    template <typename U>
    FORCE_INLINE_F void Deallocate(U *ptr)
    {
        decltype(DeallocateFunc)::operator()(ptr);
    }

    OPTIONAL_FIELD(kCondition, AllocatorT) Allocator {};
};

template <typename T>
using KMallocAllocator = CustomAllocatorWrapper<
    []<typename... Args>(this auto &self, Args &&...args) FORCE_INLINE_L {
        auto mem = Mem::KMalloc<T>();
        if (!mem) {
            return static_cast<T *>(nullptr);
        }

        return new (*mem) T(std::forward<Args>(args)...);
    },
    []<typename U>(this auto &self, U *ptr) FORCE_INLINE_L {
        if (ptr) {
            ptr->~U();
            Mem::KFree(ptr);
        }
    }>;

template <typename T, size_t kPoolSize>
using CyclicAllocatorWrapper = CustomAllocatorWrapper<
    []<typename... Args>(this auto &self, Args &&...args) FORCE_INLINE_L {
        return self.Allocator.Allocate(std::forward<Args>(args)...);
    },
    []<typename U>(this auto &self, U *ptr) FORCE_INLINE_L {
        self.Allocator.Free(ptr);
    },
    CyclicAllocator<T, kPoolSize>>;

}  // namespace allocators

#endif  // LIBS_LIBCONTAINERS_INCLUDE_ALLOCATORS_CUSTOM_ALLOCATOR_WRAPPER_HPP_
