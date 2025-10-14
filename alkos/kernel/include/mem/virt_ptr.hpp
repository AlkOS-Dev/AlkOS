#ifndef ALKOS_KERNEL_INCLUDE_MEM_VIRT_PTR_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VIRT_PTR_HPP_

#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>
#include "hal/constants.hpp"

namespace mem
{

template <class T>
class VirtualPtr
{
    public:
    //==============================================================================
    // Class Creation & Destruction
    //==============================================================================

    explicit VirtualPtr(T* virt_ptr) : virt_ptr_{virt_ptr} {}

    //==============================================================================
    // Operators
    //==============================================================================

    // Enabling pointer-like behavior

    T& operator*() const { return *virt_ptr_; }
    T* operator->() const { return virt_ptr_; }

    explicit operator bool() const { return !IsNull(); }

    // Enabling pointer arithmetic

    VirtualPtr<T> operator+(u64 offset) const { return VirtualPtr<T>(virt_ptr_ + offset); }

    VirtualPtr<T>& operator+=(u64 offset)
    {
        virt_ptr_ += offset;
        return *this;
    }

    //==============================================================================
    // Observers
    //==============================================================================

    T* Get() const { return virt_ptr_; }
    T* ToPhys() const
    {
        return reinterpret_cast<T*>(
            reinterpret_cast<uintptr_t>(virt_ptr_) - hal::kKernelVirtualAddressStart
        );
    }
    bool IsNull() const { return virt_ptr_ == nullptr; }

    private:
    T* virt_ptr_;
};

template <>
class VirtualPtr<void>
{
    public:
    //==============================================================================
    // Class Creation & Destruction
    //==============================================================================

    VirtualPtr() : virt_ptr_{nullptr} {}
    explicit VirtualPtr(void* virt_ptr) : virt_ptr_{virt_ptr} {}

    //==============================================================================
    // Operators
    //==============================================================================

    explicit operator bool() const { return !IsNull(); }

    // Enabling pointer arithmetic

    VirtualPtr<void> operator+(u64 offset) const
    {
        // `char*` for standard-compliant byte-level pointer arithmetic.
        return VirtualPtr<void>(static_cast<char*>(virt_ptr_) + offset);
    }

    VirtualPtr<void>& operator+=(u64 offset)
    {
        // `char*` for standard-compliant byte-level pointer arithmetic.
        virt_ptr_ = static_cast<char*>(virt_ptr_) + offset;
        return *this;
    }

    //==============================================================================
    // Observers
    //==============================================================================

    void* Get() const { return virt_ptr_; }
    void* ToPhys() const { return static_cast<char*>(virt_ptr_) - hal::kKernelVirtualAddressStart; }
    bool IsNull() const { return virt_ptr_ == nullptr; }

    private:
    void* virt_ptr_;
};

}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VIRT_PTR_HPP_
