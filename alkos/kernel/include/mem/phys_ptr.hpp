#ifndef ALKOS_KERNEL_INCLUDE_PHYSICAL_PTR_HPP_
#define ALKOS_KERNEL_INCLUDE_PHYSICAL_PTR_HPP_

#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>
#include <settings.hpp>

template <class T>
class PhysicalPtr
{
    public:
    //==============================================================================
    // Class Creation & Destruction
    //==============================================================================

    explicit PhysicalPtr(T* phys_ptr) : phys_ptr_{phys_ptr} {}

    //==============================================================================
    // Operators
    //==============================================================================

    // Enabling pointer-like behavior

    T& operator*() const { return *(ToVirt()); }
    T* operator->() const { return ToVirt(); }

    explicit operator bool() const { return !IsNull(); }

    // Enabling pointer arithmetic

    PhysicalPtr<T> operator+(u64 offset) const { return PhysicalPtr<T>(phys_ptr_ + offset); }

    PhysicalPtr<T>& operator+=(u64 offset)
    {
        phys_ptr_ += offset;
        return *this;
    }

    //==============================================================================
    // Observers
    //==============================================================================

    T* Get() const { return phys_ptr_; }
    T* ToVirt() const
    {
        return reinterpret_cast<T*>(
            reinterpret_cast<uintptr_t>(phys_ptr_) + kKernelVirtualAddressStart
        );
    }
    bool IsNull() const { return phys_ptr_ == nullptr; }

    private:
    T* phys_ptr_;
};

template <>
class PhysicalPtr<void>
{
    public:
    //==============================================================================
    // Class Creation & Destruction
    //==============================================================================

    PhysicalPtr() : phys_ptr_{nullptr} {}
    explicit PhysicalPtr(void* phys_ptr) : phys_ptr_{phys_ptr} {}

    //==============================================================================
    // Operators
    //==============================================================================

    explicit operator bool() const { return !IsNull(); }

    // Enabling pointer arithmetic

    PhysicalPtr<void> operator+(u64 offset) const
    {
        // `char*` for standard-compliant byte-level pointer arithmetic.
        return PhysicalPtr<void>(static_cast<char*>(phys_ptr_) + offset);
    }

    PhysicalPtr<void>& operator+=(u64 offset)
    {
        // `char*` for standard-compliant byte-level pointer arithmetic.
        phys_ptr_ = static_cast<char*>(phys_ptr_) + offset;
        return *this;
    }

    //==============================================================================
    // Observers
    //==============================================================================

    void* Get() const { return phys_ptr_; }
    void* ToVirt() const { return static_cast<char*>(phys_ptr_) + kKernelVirtualAddressStart; }
    bool IsNull() const { return phys_ptr_ == nullptr; }

    private:
    void* phys_ptr_;
};

#endif  // ALKOS_KERNEL_INCLUDE_PHYSICAL_PTR_HPP_
