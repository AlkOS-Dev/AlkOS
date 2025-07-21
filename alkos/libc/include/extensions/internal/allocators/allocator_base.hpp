#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_ALLOCATOR_BASE_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_ALLOCATOR_BASE_HPP_

#include <extensions/concepts.hpp>
#include <extensions/memory.hpp>
#include <extensions/type_traits.hpp>

struct AllocatorBlock {
    void *ptr;
    size_t size;

    constexpr AllocatorBlock(void *ptr = nullptr, size_t size = 0) : ptr(ptr), size(size) {}

    template <typename T>
    operator T *() const
    {
        return static_cast<T *>(ptr);
    }
    operator bool() const { return ptr != nullptr && size > 0; }
    bool operator==(const AllocatorBlock &other) const
    {
        return ptr == other.ptr && size == other.size;
    }
};

template <class T>
concept Allocator = requires(T a, size_t n, AllocatorBlock block) {
    { a.Allocate(n) } -> std::same_as<AllocatorBlock>;
    { a.Deallocate(block) } -> std::same_as<void>;
    { a.Owns(block) } -> std::same_as<bool>;
    { a.DeallocateAll() } -> std::same_as<void>;
};

template <typename T, typename... Args>
WRAP_CALL T *Construct(AllocatorBlock &block, Args &&...args)
{
    std::construct_at<T>(block, std::forward<Args>(args)...);
    return block;
}

template <typename T>
FAST_CALL void Destroy(AllocatorBlock &block)
{
    if constexpr (!std::is_trivially_destructible_v<T>) {
        static_cast<T *>(block)->~T();
    }
}

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_ALLOCATOR_BASE_HPP_
