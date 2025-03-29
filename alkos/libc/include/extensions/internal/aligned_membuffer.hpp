#ifndef ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALIGNED_MEMBUFFER_HPP_
#define ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALIGNED_MEMBUFFER_HPP_

#include <extensions/type_traits.hpp>

namespace internal
{

template <typename Type>
class AlignedMemoryBuffer
{
    private:
    //------------------------------------------------------------------------------//
    // Internal Types
    //------------------------------------------------------------------------------//
    using storage_t = std::aligned_storage_t<sizeof(Type), std::alignment_of_v<Type>>;

    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//
    constexpr AlignedMemoryBuffer() noexcept = default;

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//
    constexpr void* GetAddress() noexcept { return static_cast<void*>(&storage.data); }

    constexpr const void* GetAddress() const noexcept
    {
        return static_cast<const void*>(&storage.data);
    }

    constexpr Type* GetPtr() noexcept { return static_cast<Type*>(GetAddress()); }

    constexpr const Type* GetPtr() const noexcept { return static_cast<const Type*>(GetAddress()); }

    private:
    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//
    storage_t storage;
};

}  // namespace internal

#endif  // ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALIGNED_MEMBUFFER_HPP_
