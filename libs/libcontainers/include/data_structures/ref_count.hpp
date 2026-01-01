#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_REF_COUNT_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_REF_COUNT_HPP_

#include "atomic.hpp"
#include "defines.hpp"
#include "types.h"

namespace data_structures
{

/**
 * @brief Intrusive thread-safe reference counting base class
 */
template <typename T>
class RefCountedBase
{
    public:
    using CounterType = u32;

    RefCountedBase() noexcept : ref_count_(0) {}
    virtual ~RefCountedBase() = default;

    // Disable copy/move - each object has unique ref count
    RefCountedBase(const RefCountedBase &)            = delete;
    RefCountedBase &operator=(const RefCountedBase &) = delete;
    RefCountedBase(RefCountedBase &&)                 = delete;
    RefCountedBase &operator=(RefCountedBase &&)      = delete;

    // ========================================================================
    // Reference counting operations
    // ========================================================================

    void AddRef() noexcept { (void)ref_count_.fetch_add(1, std::memory_order_relaxed); }

    void Release() noexcept
    {
        CounterType old_count = ref_count_.fetch_sub(1, std::memory_order_acq_rel);

        ASSERT_GT(old_count, 0, "Releasing ref_count that is already zero");

        if (old_count == 1) {
            static_cast<T *>(this)->~T();
        }
    }

    NODISCARD CounterType GetRefCount() const noexcept
    {
        return ref_count_.load(std::memory_order_acquire);
    }

    NODISCARD bool HasRefs() const noexcept
    {
        return ref_count_.load(std::memory_order_acquire) > 0;
    }

    private:
    std::atomic<CounterType> ref_count_;
};

// ============================================================================
// Smart pointer for intrusive reference counting
// ============================================================================

template <typename T>
class RefPtr
{
    static_assert(std::derived_from<T, RefCountedBase<T>>, "T must derive from RefCountedBase<T>");

    public:
    RefPtr() noexcept : ptr_(nullptr) {}

    explicit RefPtr(T *ptr, bool add_ref = true) noexcept : ptr_(ptr)
    {
        if (ptr_ && add_ref) {
            ptr_->AddRef();
        }
    }

    RefPtr(const RefPtr &other) noexcept : ptr_(other.ptr_)
    {
        if (ptr_) {
            ptr_->AddRef();
        }
    }

    RefPtr(RefPtr &&other) noexcept : ptr_(other.ptr_) { other.ptr_ = nullptr; }

    ~RefPtr()
    {
        if (ptr_) {
            ptr_->Release();
        }
    }

    RefPtr &operator=(const RefPtr &other) noexcept
    {
        if (this != &other) {
            T *old_ptr = ptr_;
            ptr_       = other.ptr_;
            if (ptr_) {
                ptr_->AddRef();
            }
            if (old_ptr) {
                old_ptr->Release();
            }
        }
        return *this;
    }

    RefPtr &operator=(RefPtr &&other) noexcept
    {
        if (this != &other) {
            if (ptr_) {
                ptr_->Release();
            }
            ptr_       = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    RefPtr &operator=(T *ptr) noexcept
    {
        if (ptr_ != ptr) {
            T *old_ptr = ptr_;
            ptr_       = ptr;
            if (ptr_) {
                ptr_->AddRef();
            }
            if (old_ptr) {
                old_ptr->Release();
            }
        }
        return *this;
    }

    // ========================================================================
    // Accessors
    // ========================================================================

    NODISCARD T *Get() const noexcept { return ptr_; }
    NODISCARD T *operator->() const noexcept { return ptr_; }
    NODISCARD T &operator*() const noexcept { return *ptr_; }

    NODISCARD explicit operator bool() const noexcept { return ptr_ != nullptr; }

    void Reset() noexcept
    {
        if (ptr_) {
            ptr_->Release();
            ptr_ = nullptr;
        }
    }

    T *Detach() noexcept
    {
        T *ptr = ptr_;
        ptr_   = nullptr;
        return ptr;
    }

    // ========================================================================
    // Comparison operators
    // ========================================================================

    NODISCARD bool operator==(const RefPtr &other) const noexcept { return ptr_ == other.ptr_; }
    NODISCARD bool operator!=(const RefPtr &other) const noexcept { return ptr_ != other.ptr_; }
    NODISCARD bool operator==(T *ptr) const noexcept { return ptr_ == ptr; }
    NODISCARD bool operator!=(T *ptr) const noexcept { return ptr_ != ptr; }
    NODISCARD bool operator==(std::nullptr_t) const noexcept { return ptr_ == nullptr; }
    NODISCARD bool operator!=(std::nullptr_t) const noexcept { return ptr_ != nullptr; }

    private:
    T *ptr_;
};

// ============================================================================
// Helper functions
// ============================================================================

template <typename T, typename... Args>
RefPtr<T> MakeRefCounted(Args &&...args)
{
    // Note: New object starts with ref_count_ = 0, so we pass add_ref = true
    return RefPtr<T>(new T(std::forward<Args>(args)...), true);
}

template <typename T>
RefPtr<T> AdoptRef(T *ptr) noexcept
{
    // Adopt existing reference without incrementing
    return RefPtr<T>(ptr, false);
}

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_REF_COUNT_HPP_
