// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBCONTAINERS_INCLUDE_ALLOCATORS_CUSTOM_ALLOCATOR_WRAPPER_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_ALLOCATORS_CUSTOM_ALLOCATOR_WRAPPER_HPP_

#include <mem/cyclic_allocator.hpp>
#include <mem/heap.hpp>
#include <template_lib.hpp>
#include <type_traits.hpp>

namespace allocators
{

template <typename AllocateFunc, typename DeallocateFunc, typename AllocatorT = void>
class CustomAllocatorWrapper : public AllocateFunc, public DeallocateFunc
{
    static constexpr bool kCondition = !std::is_void_v<AllocatorT>;

    public:
    template <typename U, typename... Args>
    FORCE_INLINE_F U *Allocate(Args &&...args)
    {
        return AllocateFunc::template operator()<U>(std::forward<Args>(args)...);
    }

    template <typename U>
    FORCE_INLINE_F void Deallocate(U *ptr)
    {
        DeallocateFunc::operator()(ptr);
    }

    OPTIONAL_FIELD(kCondition, AllocatorT) Allocator {};
};

// Stateless functor for KMalloc
struct KMallocAllocate {
    template <typename T, typename... Args>
    static FORCE_INLINE_F T *operator()(Args &&...args)
    {
        auto mem = Mem::KMalloc<T>();
        if (!mem) {
            return static_cast<T *>(nullptr);
        }
        return new (*mem) T(std::forward<Args>(args)...);
    }
};

struct KMallocDeallocate {
    template <typename U>
    static FORCE_INLINE_F void operator()(U *ptr)
    {
        if (ptr) {
            ptr->~U();
            Mem::KFree(ptr);
        }
    }
};

template <typename T>
using KMallocAllocator = CustomAllocatorWrapper<KMallocAllocate, KMallocDeallocate>;

template <typename T, size_t kSize>
class CyclicAllocatorWrapper
{
    CyclicAllocator<T, kSize> allocator_;

    public:
    template <typename U, typename... Args>
    U *Allocate(Args &&...args)
    {
        static_assert(
            std::is_same_v<U, T>, "CyclicAllocatorWrapper can only allocate its underlying type"
        );
        return allocator_.Allocate(std::forward<Args>(args)...);
    }

    template <typename U>
    void Deallocate(U *ptr)
    {
        static_assert(
            std::is_same_v<U, T>, "CyclicAllocatorWrapper can only deallocate its underlying type"
        );
        allocator_.Free(ptr);
    }
};

}  // namespace allocators

#endif  // LIBS_LIBCONTAINERS_INCLUDE_ALLOCATORS_CUSTOM_ALLOCATOR_WRAPPER_HPP_
