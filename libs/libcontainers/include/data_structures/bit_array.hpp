#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_BIT_ARRAY_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_BIT_ARRAY_HPP_

#include <stddef.h>
#include <optional.hpp>
#include "assert.h"
#include "bit.hpp"
#include "string.h"
#include "types.hpp"

// TODO: Minimal changes would be required to make this
// https://en.cppreference.com/w/cpp/utility/bitset.html

//==============================================================================
// BitMapView - Non-owning view with runtime size
//==============================================================================

namespace data_structures
{

class BitMapView final
{
    using StorageT                        = u8;
    static constexpr size_t kStorageTBits = sizeof(StorageT) * 8;

    public:
    //==============================================================================
    // Public Methods
    //==============================================================================

    BitMapView(void *storage_ptr, const size_t num_bits)
        : storage_ptr_{static_cast<StorageT *>(storage_ptr)}, num_bits_{num_bits}
    {
    }

    FORCE_INLINE_F void SetTrue(const size_t index)
    {
        ASSERT_LT(index, num_bits_, "Index out of bounds");
        storage_ptr_[index / kStorageTBits] |= (kLsb<StorageT> << (index % kStorageTBits));
    }

    FORCE_INLINE_F void SetFalse(const size_t index)
    {
        ASSERT_LT(index, num_bits_, "Index out of bounds");
        storage_ptr_[index / kStorageTBits] &= ~(kLsb<StorageT> << (index % kStorageTBits));
    }

    FORCE_INLINE_F void Set(const size_t index, const bool value)
    {
        ASSERT_LT(index, num_bits_, "Index out of bounds");
        storage_ptr_[index / kStorageTBits] =
            (storage_ptr_[index / kStorageTBits] & ~(kLsb<StorageT> << (index % kStorageTBits))) |
            (static_cast<StorageT>(value) << (index % kStorageTBits));
    }

    NODISCARD FORCE_INLINE_F bool Get(const size_t index) const
    {
        ASSERT_LT(index, num_bits_, "Index out of bounds");
        return (storage_ptr_[index / kStorageTBits] >> (index % kStorageTBits)) & kLsb<StorageT>;
    }

    FORCE_INLINE_F void SetAll(const bool value)
    {
        const size_t numStorageT = (num_bits_ + kStorageTBits - 1) / kStorageTBits;
        memset(storage_ptr_, value ? 0xFF : 0, numStorageT * sizeof(StorageT));
    }

    NODISCARD FORCE_INLINE_F size_t Size() const { return num_bits_; }

    NODISCARD FORCE_INLINE_F StorageT *Storage() { return storage_ptr_; }
    NODISCARD FORCE_INLINE_F const StorageT *Storage() const { return storage_ptr_; }

    private:
    StorageT *storage_ptr_;
    size_t num_bits_;
};

//==============================================================================
// BitArray - Owning array with compile-time size
//==============================================================================

template <size_t kNumBits>
class PACK BitArray final
{
    using StorageT                       = u8;
    static constexpr size_t kStorageBits = sizeof(StorageT) * 8;

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    BitArray()  = default;
    ~BitArray() = default;

    // ------------------------------
    // Class methods
    // ------------------------------

    FORCE_INLINE_F void SetTrue(const size_t index)
    {
        ASSERT_LT(index, kNumBits, "Index out of bounds in BitArray");

        storage_[index / kStorageBits] |= (kLsb<StorageT> << (index % kStorageBits));
    }

    FORCE_INLINE_F void SetFalse(const size_t index)
    {
        ASSERT_LT(index, kNumBits, "Index out of bounds in BitArray");

        storage_[index / kStorageBits] &= ~(kLsb<StorageT> << (index % kStorageBits));
    }

    FORCE_INLINE_F void Set(const size_t index, const bool value)
    {
        ASSERT_LT(index, kNumBits, "Index out of bounds in BitArray");

        SetFalse(index);
        storage_[index / kStorageBits] |= (static_cast<u32>(value) << (index % kStorageBits));
    }

    NODISCARD FORCE_INLINE_F bool Get(const size_t index) const
    {
        ASSERT_LT(index, kNumBits, "Index out of bounds in BitArray");
        return (storage_[index / kStorageBits] >> (index % kStorageBits)) & kLsb<StorageT>;
    }

    template <typename T, size_t Offset, size_t Width>
    NODISCARD FORCE_INLINE_F T GetRange() const
    {
        static_assert(Offset + Width <= kNumBits, "Range exceeds BitArray size");

        constexpr size_t byte_offset = Offset / 8;
        constexpr size_t bit_offset  = Offset % 8;

        if constexpr (bit_offset + Width <= 64) {
            u64 raw = *reinterpret_cast<const u64 *>(storage_ + byte_offset);

            // Shift right to align, then mask
            constexpr u64 mask = kBitMaskRight<u64, Width>;
            return static_cast<T>((raw >> bit_offset) & mask);
        } else {
            // Spans two 64-bit words
            constexpr size_t first_width  = 64 - bit_offset;
            constexpr size_t second_width = Width - first_width;

            // Read first word
            u64 raw1            = *reinterpret_cast<const u64 *>(storage_ + byte_offset);
            constexpr u64 mask1 = kBitMaskRight<u64, first_width>;
            T result            = static_cast<T>((raw1 >> bit_offset) & mask1);

            // Read second word
            u64 raw2            = *reinterpret_cast<const u64 *>(storage_ + byte_offset + 8);
            constexpr u64 mask2 = kBitMaskRight<u64, second_width>;
            result |= static_cast<T>((raw2 & mask2) << first_width);

            return result;
        }
    }

    template <typename T, size_t Offset, size_t Width>
    FORCE_INLINE_F void SetRange(T value)
    {
        static_assert(Offset + Width <= kNumBits, "Range exceeds BitArray size");

        // Mask value to width
        constexpr T value_mask = kBitMaskRight<T, Width>;
        value &= value_mask;

        constexpr size_t byte_offset = Offset / 8;
        constexpr size_t bit_offset  = Offset % 8;

        if constexpr (bit_offset + Width <= 64) {
            auto *raw_ptr = reinterpret_cast<u64 *>(storage_ + byte_offset);
            u64 raw       = *raw_ptr;

            // Create clear mask at the correct position
            constexpr u64 clear_mask = kBitMaskRight<u64, Width> << bit_offset;

            // Clear bits: use ~mask & raw
            raw &= ~clear_mask;

            // Set new bits: shift value and OR
            raw |= (static_cast<u64>(value) << bit_offset);

            // Write back
            *raw_ptr = raw;
        } else {
            // Spans two 64-bit words - split the operation
            constexpr size_t first_width  = 64 - bit_offset;
            constexpr size_t second_width = Width - first_width;

            // First word: clear and set lower bits
            auto *raw_ptr1            = reinterpret_cast<u64 *>(storage_ + byte_offset);
            u64 raw1                  = *raw_ptr1;
            constexpr u64 clear_mask1 = kBitMaskRight<u64, first_width> << bit_offset;
            raw1 &= ~clear_mask1;
            raw1 |= (static_cast<u64>(value) << bit_offset);
            *raw_ptr1 = raw1;

            // Second word: clear and set upper bits
            auto *raw_ptr2            = reinterpret_cast<u64 *>(storage_ + byte_offset + 8);
            u64 raw2                  = *raw_ptr2;
            constexpr u64 clear_mask2 = kBitMaskRight<u64, second_width>;
            raw2 &= ~clear_mask2;
            raw2 |= (static_cast<u64>(value) >> first_width);
            *raw_ptr2 = raw2;
        }
    }

    NODISCARD FORCE_INLINE_F size_t Size() const { return kNumBits; }

    FORCE_INLINE_F void SetAll(const bool value)
    {
        memset(storage_, value ? UINT8_MAX : 0, kNumStorage * sizeof(StorageT));
    }

    FORCE_INLINE_F u64 ToU64() const
    {
        ASSERT_LE(kNumBits, 64_size, "ToU64 supported only for small enough arrays");
        return *reinterpret_cast<const u64 *>(storage_);
    }

    FORCE_INLINE_F u32 ToU32() const
    {
        ASSERT_LE(kNumBits, 32_size, "ToU32 supported only for small enough arrays");
        return *reinterpret_cast<const u32 *>(storage_);
    }

    // std::numeric_limits<size_t>::max() for failure
    template <bool value = false>
    NODISCARD FORCE_INLINE_F size_t FindFirst()
    {
        const StorageT empty_unit =
            value ? std::numeric_limits<StorageT>::min() : std::numeric_limits<StorageT>::max();

        for (size_t i = 0; i < kNumStorage; ++i) {
            if (storage_[i] == empty_unit) {
                continue;
            }

            const size_t start = i * kStorageBits;
            const size_t local_offset =
                value ? std::countr_zero(storage_[i]) : std::countr_one(storage_[i]);
            return start + local_offset;
        }

        return std::numeric_limits<size_t>::max();
    }

    NODISCARD FORCE_INLINE_F size_t FindFirst(const bool value = false)
    {
        if (value) {
            return FindFirst<true>();
        }
        return FindFirst<false>();
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr size_t kNumStorage = (kNumBits + kStorageBits - 1) / kStorageBits;

    private:
    StorageT storage_[kNumStorage]{};
};

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_BIT_ARRAY_HPP_
