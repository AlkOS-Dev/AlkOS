#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_ARRAY_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_ARRAY_HPP_

#include <stddef.h>
#include "assert.h"
#include "bit.hpp"
#include "types.hpp"

template <size_t kNumBits>
class BitArray final
{
    using StorageT                        = u32;
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
        memset(storage_, value ? UINT8_MAX : 0, kNumStorageT);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    StorageT storage_[kNumStorageT]{};
};

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_ARRAY_HPP_
