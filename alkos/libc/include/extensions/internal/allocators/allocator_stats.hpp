#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_ALLOCATOR_STATS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_ALLOCATOR_STATS_HPP_

#include <extensions/defines.hpp>
#include <extensions/flags.hpp>
#include <extensions/internal/allocators/affix_allocator.hpp>
#include <extensions/source_location.hpp>

namespace internal
{
template <flags::AllocatorStats_t Flags>
struct GlobalAllocatorStats {
    NO_UNIQUE_ADDRESS
    std::conditional_t<Flags & flags::AllocatorStats::Allocations, size_t, UNIQUE_EMPTY>
        allocations{};
    NO_UNIQUE_ADDRESS
    std::conditional_t<Flags & flags::AllocatorStats::Allocations, size_t, UNIQUE_EMPTY>
        bytes_allocated{};

    NO_UNIQUE_ADDRESS
    std::conditional_t<Flags & flags::AllocatorStats::Deallocations, size_t, UNIQUE_EMPTY>
        deallocations{};
    NO_UNIQUE_ADDRESS
    std::conditional_t<Flags & flags::AllocatorStats::Deallocations, size_t, UNIQUE_EMPTY>
        bytes_deallocated{};
};

template <flags::AllocatorStats_t Flags>
struct PerAllocationStats {
    void* ptr;
    size_t size;
    std::source_location loc;
    PerAllocationStats* next;
    PerAllocationStats* prev;
};

template <flags::AllocatorStats_t Flags>
using PerAllocationStats_t =
    std::conditional_t<Flags & flags::AllocatorStats::DebugInfo, PerAllocationStats<Flags>, void>;

template <Allocator T, flags::AllocatorStats_t Flags>
using BaseAllocator = AffixAllocator<T, PerAllocationStats_t<Flags>>;
}  // namespace internal

template <Allocator T, flags::AllocatorStats_t Flags = {}>
class AllocatorStats : private internal::BaseAllocator<T, Flags>
{
    using BaseAllocator = internal::BaseAllocator<T, Flags>;
    using GlobalStats   = internal::GlobalAllocatorStats<Flags>;
    using Stats         = internal::PerAllocationStats<Flags>;

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    AllocatorStats()  = default;
    ~AllocatorStats() = default;

    // ------------------------------
    // Class methods
    // ------------------------------

    NODISCARD constexpr AllocatorBlock Allocate(
        size_t n, std::source_location loc = std::source_location::current()
    )
    {
        auto block = BaseAllocator::Allocate(n);
        if (block.ptr) {
            if constexpr (Flags & flags::AllocatorStats::Allocations) {
                ++global_stats_.allocations;
                global_stats_.bytes_allocated += n;
            }
            if constexpr (Flags & flags::AllocatorStats::DebugInfo) {
                Stats* stats = reinterpret_cast<Stats*>(
                    static_cast<byte*>(block.ptr) - BaseAllocator::prefix_size
                );
                stats->ptr  = block.ptr;
                stats->size = n;
                stats->loc  = loc;

                stats->next = allocation_head_;
                stats->prev = nullptr;
                if (allocation_head_) {
                    allocation_head_->prev = stats;
                }
                allocation_head_ = stats;
            }
        }
        return block;
    }

    constexpr void Deallocate(AllocatorBlock block)
    {
        if (block.ptr == nullptr || block.size == 0) {
            return;
        }

        if constexpr (Flags & flags::AllocatorStats::Deallocations) {
            ++global_stats_.deallocations;
            global_stats_.bytes_deallocated += block.size;
        }

        if constexpr (Flags & flags::AllocatorStats::DebugInfo) {
            Stats* stats = reinterpret_cast<Stats*>(
                static_cast<byte*>(block.ptr) - BaseAllocator::prefix_size
            );

            if (stats->prev) {
                stats->prev->next = stats->next;
            } else {
                allocation_head_ = stats->next;
            }
            if (stats->next) {
                stats->next->prev = stats->prev;
            }
        }

        BaseAllocator::Deallocate(block);
    }

    FORCE_INLINE_F constexpr void DeallocateAll()
    {
        if constexpr (Flags & flags::AllocatorStats::Deallocations) {
            global_stats_.deallocations     = global_stats_.allocations;
            global_stats_.bytes_deallocated = global_stats_.bytes_allocated;
        }
        BaseAllocator::DeallocateAll();
    }

    NODISCARD FORCE_INLINE_F constexpr bool Owns(AllocatorBlock block) const
    {
        return BaseAllocator::Owns(block);
    }

    NODISCARD FORCE_INLINE_F constexpr const GlobalStats& GetGlobalStats() const
    {
        return global_stats_;
    }

    template <flags::AllocatorStats_t F = Flags>
    NODISCARD FORCE_INLINE_F constexpr std::enable_if_t<
        F & flags::AllocatorStats::DebugInfo, const Stats&>
    GetAllocationInfo(AllocatorBlock block) const
    {
        return *reinterpret_cast<Stats*>(
            static_cast<byte*>(block.ptr) - BaseAllocator::prefix_size
        );
    }

    template <flags::AllocatorStats_t F = Flags>
    NODISCARD FORCE_INLINE_F constexpr std::enable_if_t<
        F & flags::AllocatorStats::DebugInfo, const Stats*>
    GetAllocations() const
    {
        return allocation_head_;
    }

    private:
    NO_UNIQUE_ADDRESS GlobalStats global_stats_;
    NO_UNIQUE_ADDRESS
    std::conditional_t<Flags & flags::AllocatorStats::DebugInfo, Stats*, UNIQUE_EMPTY>
        allocation_head_{};
};

#include <extensions/internal/allocators/stub_allocator.hpp>

static_assert(
    Allocator<AllocatorStats<
        StubAllocator<>,
        flags::AllocatorStats::Allocations | flags::AllocatorStats::Deallocations>>,
    "AllocatorStats must be an Allocator"
);

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_ALLOCATOR_STATS_HPP_
