#ifndef KERNEL_SRC_MEM_PHYS_MNGR_SLAB_EFFICIENCY_HPP_
#define KERNEL_SRC_MEM_PHYS_MNGR_SLAB_EFFICIENCY_HPP_

#include <array.hpp>
#include <limits.hpp>
#include <utility.hpp>

#include <stdio.h>
#include "trace_framework.hpp"

#include "mem/phys/mngr/buddy.hpp"

namespace Mem::SlabEfficiency
{

static constexpr f64 kEfficiencyTolerance = 0.01F;  // Max efficiency is 1.0

template <size_t ObjectSize, u8 BlockOrder>
struct SlabEfficiencyInfo {
    static constexpr size_t kBlockSize = BuddyPmm::BuddyAreaSize(BlockOrder);

    using FreeListItemType = decltype([] {
        if constexpr (kBlockSize / (ObjectSize + sizeof(u8)) < std::numeric_limits<u8>::max()) {
            return u8{};
        } else if constexpr (kBlockSize / (ObjectSize + sizeof(u16)) <
                             std::numeric_limits<u16>::max()) {
            return u16{};
        } else if constexpr (kBlockSize / (ObjectSize + sizeof(u32)) <
                             std::numeric_limits<u32>::max()) {
            return u32{};
        } else {
            return u64{};
        }
    }());

    static constexpr size_t kSizeOfMetadataPerObj   = sizeof(FreeListItemType);
    static constexpr size_t kTotalStoredBytesPerObj = ObjectSize + kSizeOfMetadataPerObj;

    static constexpr size_t kCapacity =
        (kTotalStoredBytesPerObj > 0) ? (kBlockSize / kTotalStoredBytesPerObj) : 0;
    static constexpr f64 kSpaceEfficiency =
        (kBlockSize > 0) ? static_cast<f64>(kCapacity * ObjectSize) / kBlockSize : 0.0;
};

struct EfficiencyData {
    f64 efficiency;
    u8 block_order;

    /// @brief We want to sort by efficiency first (within given tolerance), then by block order.
    /// @param other Other efficiency data to compare with.
    /// @return True if this is less than other, false otherwise.
    constexpr bool operator<(const EfficiencyData &other) const
    {
        const f64 diff     = efficiency - other.efficiency;
        const f64 abs_diff = (diff > 0) ? diff : -diff;

        if (abs_diff <= kEfficiencyTolerance) {
            return block_order > other.block_order;
        } else {
            return efficiency < other.efficiency;
        }
    }
};

template <typename T, size_t N>
constexpr std::array<T, N> InsertionSort(std::array<T, N> arr)
{
    for (size_t i = 1; i < N; ++i) {
        T key    = arr[i];
        size_t j = i;
        while (j > 0 && arr[j - 1] < key) {
            arr[j] = arr[j - 1];
            --j;
        }
        arr[j] = key;
    }
    return arr;
}

constexpr size_t kMinObjectSizeLog2 = 3;   // 2^3 = 8
constexpr size_t kMaxObjectSizeLog2 = 12;  // 2^12 = 4096
constexpr size_t kNumObjectSizes    = kMaxObjectSizeLog2 - kMinObjectSizeLog2 + 1;
constexpr u8 kNumBlockOrders        = BuddyPmm::kMaxOrder + 1;

template <size_t ObjectSize, size_t... BlockOrders>
constexpr std::array<u8, kNumBlockOrders> GenerateSortedBlockOrdersImpl(
    std::index_sequence<BlockOrders...>
)
{
    std::array<EfficiencyData, kNumBlockOrders> efficiencies = {
        {{SlabEfficiencyInfo<ObjectSize, BlockOrders>::kSpaceEfficiency,
          static_cast<u8>(BlockOrders)}...}
    };

    auto sorted_efficiencies = InsertionSort(efficiencies);

    std::array<u8, kNumBlockOrders> sorted_orders{};
    for (size_t i = 0; i < kNumBlockOrders; ++i) {
        sorted_orders[i] = sorted_efficiencies[i].block_order;
    }
    return sorted_orders;
}

template <size_t ObjectSize>
constexpr std::array<u8, kNumBlockOrders> GenerateSortedBlockOrders()
{
    return GenerateSortedBlockOrdersImpl<ObjectSize>(std::make_index_sequence<kNumBlockOrders>());
}

template <size_t... Is>
constexpr std::array<std::array<u8, kNumBlockOrders>, sizeof...(Is)> GenerateEfficiencyTableImpl(
    std::index_sequence<Is...>
)
{
    return {{GenerateSortedBlockOrders<(1ULL << (Is + kMinObjectSizeLog2))>()...}};
}

constexpr auto GenerateEfficiencyTable()
{
    return GenerateEfficiencyTableImpl(std::make_index_sequence<kNumObjectSizes>());
}

inline constexpr auto kSlabEfficiencyTable = GenerateEfficiencyTable();

inline void PrintSlabEfficiencyTable()
{
    TRACE_INFO_MEMORY("Slab Efficiency Table:");
    char buffer[128];

    for (size_t i = 0; i < kNumObjectSizes; ++i) {
        const size_t object_size = 1ULL << (i + kMinObjectSizeLog2);

        int offset = snprintf(buffer, sizeof(buffer), "  Obj size: %4zu | Orders: ", object_size);

        for (size_t j = 0; j < kNumBlockOrders; ++j) {
            if (offset >= sizeof(buffer)) {
                break;
            }
            offset += snprintf(
                buffer + offset, sizeof(buffer) - offset, "%2u ", kSlabEfficiencyTable[i][j]
            );
        }
        TRACE_INFO_MEMORY("%s", buffer);
    }
}

}  // namespace Mem::SlabEfficiency

#endif  // KERNEL_SRC_MEM_PHYS_MNGR_SLAB_EFFICIENCY_HPP_
