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

    template <size_t Offset, size_t Width>
    NODISCARD FORCE_INLINE_F size_t GetRange() const
    {
        constexpr size_t kSizeTBits = sizeof(size_t) * 8;
        static_assert(Offset + Width <= kNumBits, "Range exceeds BitArray size");
        static_assert(Width <= kSizeTBits, "Width exceeds size_t size");

        constexpr size_t byte_offset = Offset / 8;
        constexpr size_t bit_offset  = Offset % 8;

        if constexpr (bit_offset + Width <= kSizeTBits) {
            size_t raw = *reinterpret_cast<const size_t *>(storage_ + byte_offset);

            // Shift right to align, then mask
            constexpr size_t mask = kBitMaskRight<size_t, Width>;
            return (raw >> bit_offset) & mask;
        } else {
            // Spans two register-sized words
            constexpr size_t first_width  = kSizeTBits - bit_offset;
            constexpr size_t second_width = Width - first_width;

            // Read first word
            size_t raw1            = *reinterpret_cast<const size_t *>(storage_ + byte_offset);
            constexpr size_t mask1 = kBitMaskRight<size_t, first_width>;
            size_t result          = (raw1 >> bit_offset) & mask1;

            // Read second word
            size_t raw2 =
                *reinterpret_cast<const size_t *>(storage_ + byte_offset + sizeof(size_t));
            constexpr size_t mask2 = kBitMaskRight<size_t, second_width>;
            result |= (raw2 & mask2) << first_width;

            return result;
        }
    }

    template <size_t Offset, size_t Width>
    FORCE_INLINE_F void SetRange(size_t value)
    {
        constexpr size_t kSizeTBits = sizeof(size_t) * 8;
        static_assert(Offset + Width <= kNumBits, "Range exceeds BitArray size");
        static_assert(Width <= kSizeTBits, "Width exceeds size_t size");

        // Mask value to width
        constexpr size_t value_mask = kBitMaskRight<size_t, Width>;
        value &= value_mask;

        constexpr size_t byte_offset = Offset / 8;
        constexpr size_t bit_offset  = Offset % 8;

        if constexpr (bit_offset + Width <= kSizeTBits) {
            auto *raw_ptr = reinterpret_cast<size_t *>(storage_ + byte_offset);
            size_t raw    = *raw_ptr;

            // Create clear mask at the correct position
            constexpr size_t clear_mask = kBitMaskRight<size_t, Width> << bit_offset;

            // Clear bits: use ~mask & raw
            raw &= ~clear_mask;

            // Set new bits: shift value and OR
            raw |= (value << bit_offset);

            // Write back
            *raw_ptr = raw;
        } else {
            // Spans two register-sized words - split the operation
            constexpr size_t first_width  = kSizeTBits - bit_offset;
            constexpr size_t second_width = Width - first_width;

            // First word: clear and set lower bits
            auto *raw_ptr1               = reinterpret_cast<size_t *>(storage_ + byte_offset);
            size_t raw1                  = *raw_ptr1;
            constexpr size_t clear_mask1 = kBitMaskRight<size_t, first_width> << bit_offset;
            raw1 &= ~clear_mask1;
            raw1 |= (value << bit_offset);
            *raw_ptr1 = raw1;

            // Second word: clear and set upper bits
            auto *raw_ptr2 = reinterpret_cast<size_t *>(storage_ + byte_offset + sizeof(size_t));
            size_t raw2    = *raw_ptr2;
            constexpr size_t clear_mask2 = kBitMaskRight<size_t, second_width>;
            raw2 &= ~clear_mask2;
            raw2 |= (value >> first_width);
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
