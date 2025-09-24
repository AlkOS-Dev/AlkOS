#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_ARRAY_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_ARRAY_HPP_

#include <stddef.h>
#include "assert.h"
#include "bit.hpp"
#include "string.h"
#include "types.hpp"

// TODO: Minimal changes would be required to make this
// https://en.cppreference.com/w/cpp/utility/bitset.html

//==============================================================================
// BitArrayBase
//==============================================================================

template <class DerivedT>
class BitArrayBase
{
    protected:
    using StorageT                        = u8;
    static constexpr size_t kStorageTBits = sizeof(StorageT) * 8;

    //==============================================================================
    // Accessors to DerivedT
    //==============================================================================

    FORCE_INLINE_F DerivedT* Derived() { return static_cast<DerivedT*>(this); }
    FORCE_INLINE_F const DerivedT* Derived() const { return static_cast<const DerivedT*>(this); }

    public:
    //==============================================================================
    // Public Methods
    //==============================================================================

    FORCE_INLINE_F void SetTrue(const size_t index)
    {
        ASSERT_LT(index, Derived()->Size(), "Index out of bounds");
        Derived()->Storage()[index / kStorageTBits] |= (kLsb<StorageT> << (index % kStorageTBits));
    }

    FORCE_INLINE_F void SetFalse(const size_t index)
    {
        ASSERT_LT(index, Derived()->Size(), "Index out of bounds");
        Derived()->Storage()[index / kStorageTBits] &= ~(kLsb<StorageT> << (index % kStorageTBits));
    }

    FORCE_INLINE_F void Set(const size_t index, const bool value)
    {
        ASSERT_LT(index, Derived()->Size(), "Index out of bounds");
        Derived()->Storage()[index / kStorageTBits] =
            (Derived()->Storage()[index / kStorageTBits] &
             ~(kLsb<StorageT> << (index % kStorageTBits))) |
            (static_cast<StorageT>(value) << (index % kStorageTBits));
    }

    NODISCARD FORCE_INLINE_F bool Get(const size_t index) const
    {
        ASSERT_LT(index, Derived()->Size(), "Index out of bounds");
        return (Derived()->Storage()[index / kStorageTBits] >> (index % kStorageTBits)) &
               kLsb<StorageT>;
    }

    FORCE_INLINE_F void SetAll(const bool value)
    {
        const size_t numStorageT = (Derived()->Size() + kStorageTBits - 1) / kStorageTBits;
        memset(Derived()->Storage(), value ? 0xFF : 0, numStorageT * sizeof(StorageT));
    }

    FORCE_INLINE_F u64 ToU64() const
    {
        ASSERT_LE(Derived()->Size(), 64, "ToU64 supported only for small enough arrays");
        return *reinterpret_cast<const u64*>(Derived()->Storage());
    }

    FORCE_INLINE_F u32 ToU32() const
    {
        ASSERT_LE(Derived()->Size(), 32, "ToU32 supported only for small enough arrays");
        return *reinterpret_cast<const u32*>(Derived()->Storage());
    }
};

//==============================================================================
// BitMapView - Non-owning view with runtime size
//==============================================================================
class BitMapView final : public BitArrayBase<BitMapView>
{
    friend class BitArrayBase<BitMapView>;

    public:
    BitMapView(void* storage_ptr, size_t num_bits)
        : storage_ptr_{reinterpret_cast<StorageT*>(storage_ptr)}, num_bits_{num_bits}
    {
    }

    //==============================================================================
    // Implementation details for base class
    //==============================================================================

    NODISCARD FORCE_INLINE_F size_t Size() const { return num_bits_; }

    protected:
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
class BitArray final : public BitArrayBase<BitArray<kNumBits>>
{
    friend class BitArrayBase<BitArray<kNumBits>>;

    public:
    BitArray() = default;

    //==============================================================================
    // Implementation details for base class
    //==============================================================================

    NODISCARD FORCE_INLINE_F constexpr size_t Size() const { return kNumBits; }

    protected:
    NODISCARD FORCE_INLINE_F typename BitArrayBase<BitArray<kNumBits>>::StorageT* Storage()
    {
        return storage_data_;
    }
    NODISCARD FORCE_INLINE_F const typename BitArrayBase<BitArray<kNumBits>>::StorageT*
    Storage() const
    {
        return storage_data_;
    }

    private:
    using Base = BitArrayBase<BitArray<kNumBits>>;
    static constexpr size_t kNumStorageT =
        (kNumBits + Base::kStorageTBits - 1) / Base::kStorageTBits;

    typename Base::StorageT storage_data_[kNumStorageT]{};
};

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_ARRAY_HPP_
