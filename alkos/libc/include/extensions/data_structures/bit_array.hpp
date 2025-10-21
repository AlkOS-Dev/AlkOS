#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_BIT_ARRAY_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_BIT_ARRAY_HPP_

#include <stddef.h>
#include "assert.h"
#include "extensions/bit.hpp"
#include "extensions/types.hpp"
#include "string.h"

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

    BitMapView(void* storage_ptr, const size_t num_bits)
        : storage_ptr_{static_cast<StorageT*>(storage_ptr)}, num_bits_{num_bits}
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

    NODISCARD FORCE_INLINE_F StorageT* Storage() { return storage_ptr_; }
    NODISCARD FORCE_INLINE_F const StorageT* Storage() const { return storage_ptr_; }

    private:
    StorageT* storage_ptr_;
    size_t num_bits_;
};

//==============================================================================
// BitArray - Owning array with compile-time size
//==============================================================================

template <size_t kNumBits>
class PACK BitArray final
{
    using StorageT                        = u8;
    static constexpr size_t kStorageTBits = sizeof(StorageT) * 8;
    static constexpr size_t kNumStorageT  = (kNumBits + kStorageTBits - 1) / kStorageTBits;

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

        storage_[index / kStorageTBits] |= (kLsb<StorageT> << (index % kStorageTBits));
    }

    FORCE_INLINE_F void SetFalse(const size_t index)
    {
        ASSERT_LT(index, kNumBits, "Index out of bounds in BitArray");

        storage_[index / kStorageTBits] &= ~(kLsb<StorageT> << (index % kStorageTBits));
    }

    FORCE_INLINE_F void Set(const size_t index, const bool value)
    {
        ASSERT_LT(index, kNumBits, "Index out of bounds in BitArray");

        SetFalse(index);
        storage_[index / kStorageTBits] |= (static_cast<u32>(value) << (index % kStorageTBits));
    }

    NODISCARD FORCE_INLINE_F bool Get(const size_t index) const
    {
        ASSERT_LT(index, kNumBits, "Index out of bounds in BitArray");
        return (storage_[index / kStorageTBits] >> (index % kStorageTBits)) & kLsb<StorageT>;
    }

    NODISCARD FORCE_INLINE_F size_t Size() const { return kNumBits; }

    FORCE_INLINE_F void SetAll(const bool value)
    {
        memset(storage_, value ? UINT8_MAX : 0, kNumStorageT * sizeof(StorageT));
    }

    FORCE_INLINE_F u64 ToU64() const
    {
        ASSERT_LE(kNumBits, 64_size, "ToU64 supported only for small enough arrays");
        return *reinterpret_cast<const u64*>(storage_);
    }

    FORCE_INLINE_F u32 ToU32() const
    {
        ASSERT_LE(kNumBits, 32_size, "ToU32 supported only for small enough arrays");
        return *reinterpret_cast<const u32*>(storage_);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    StorageT storage_[kNumStorageT]{};
};

}  // namespace data_structures

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_BIT_ARRAY_HPP_
