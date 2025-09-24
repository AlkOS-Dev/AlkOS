#ifndef ALKOS_BOOT_LIB_MEM_PHYSICAL_PTR_HPP_
#define ALKOS_BOOT_LIB_MEM_PHYSICAL_PTR_HPP_

#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>

template <class T>
class PhysicalPtr
{
    public:
    //==============================================================================
    // Class Creation & Destruction
    //==============================================================================

    PhysicalPtr() : address_{0} {}
    explicit PhysicalPtr(u64 address) : address_{address} {}

    //==============================================================================
    // Operators
    //==============================================================================

    // Enabling pointer-like behavior

    T& operator*() const { return *reinterpret_cast<T*>(static_cast<uptr>(address_)); }

    std::add_lvalue_reference_t<T> operator->() const
    {
        return reinterpret_cast<T*>(static_cast<uptr>(address_));
    }

    explicit operator bool() const { return !IsNull(); }

    // Enabling pointer arithmetic

    PhysicalPtr<T> operator+(u64 offset) const { return PhysicalPtr<T>(address_ + offset); }

    PhysicalPtr<T>& operator+=(u64 offset)
    {
        address_ += offset;
        return *this;
    }

    //==============================================================================
    // Observers
    //==============================================================================

    u64 Value() const { return address_; }

    bool IsNull() const { return address_ == 0; }

    private:
    u64 address_{};
};

template <>
class PhysicalPtr<void>
{
    public:
    //==============================================================================
    // Class Creation & Destruction
    //==============================================================================

    PhysicalPtr() : address_{0} {}
    explicit PhysicalPtr(u64 address) : address_{address} {}

    //==============================================================================
    // Operators
    //==============================================================================

    explicit operator bool() const { return !IsNull(); }

    // Enabling pointer arithmetic

    PhysicalPtr<void> operator+(u64 offset) const { return PhysicalPtr<void>(address_ + offset); }

    PhysicalPtr<void>& operator+=(u64 offset)
    {
        address_ += offset;
        return *this;
    }

    //==============================================================================
    // Observers
    //==============================================================================

    u64 Value() const { return address_; }

    bool IsNull() const { return address_ == 0; }

    private:
    u64 address_{};
};

#endif  // ALKOS_BOOT_LIB_MEM_PHYSICAL_PTR_HPP_
