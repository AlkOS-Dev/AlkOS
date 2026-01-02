#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_REF_COUNT_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_REF_COUNT_HPP_

#include "atomic.hpp"
#include "defines.hpp"
#include "mem/heap.hpp"
#include "template/utils.hpp"
#include "types.h"

namespace data_structures
{

/**
 * @brief Intrusive thread-safe reference counting base class
 */
template <typename T, bool kCustomDeleter = false>
class RefCounted
{
    using DeleterCallback = void (*)(T *);
    using CounterType     = u32;

    public:
    RefCounted() {}
    ~RefCounted() = default;

    RefCounted(const RefCounted &)            = delete;
    RefCounted &operator=(const RefCounted &) = delete;
    RefCounted(RefCounted &&)                 = delete;
    RefCounted &operator=(RefCounted &&)      = delete;

    // ========================================================================
    // Reference counting operations
    // ========================================================================

    void AddRef() noexcept { (void)ref_count_.fetch_add(1, std::memory_order_relaxed); }

    void Release() noexcept
    {
        CounterType old_count = ref_count_.fetch_sub(1, std::memory_order_acq_rel);

        ASSERT_GT(old_count, 0, "Releasing ref_count that is already zero");

        if (old_count == 1) {
            T *ptr = static_cast<T *>(this);
            ptr->~T();
            if constexpr (kCustomDeleter) {
                if (deleter_) {
                    deleter_(ptr);
                }
            }
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

    void SetDeleter(DeleterCallback deleter) noexcept
        requires(kCustomDeleter)
    {
        deleter_ = deleter;
    }

    private:
    OPTIONAL_FIELD(kCustomDeleter, DeleterCallback) deleter_ {};
    std::atomic<CounterType> ref_count_{0};
};

/**
 * @brief Smart pointer for RefCounted objects
 */
template <typename T>
class RefPtr
{
    static_assert(
        std::derived_from<T, RefCounted<T, false>> || std::derived_from<T, RefCounted<T, true>>,
        "T must derive from RefCountedBase<T>"
    );

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

template <
    typename T, typename Allocator = decltype([]<typename... Args>(Args &&...args) {
                    return Mem::KNew<T>(std::forward<Args>(args)...).value_or(nullptr);
                }),
    typename Deleter = decltype([](T *ptr) {
        Mem::KDelete(ptr);
    }),
    typename... Args>
RefPtr<T> MakeRefCounted(Args &&...args)
    requires(std::derived_from<T, RefCounted<T, true>>)
{
    auto res = Allocator{}(std::forward<Args>(args)...);
    if (!res) {
        return RefPtr<T>{};
    }
    res->SetDeleter(Deleter{});
    return RefPtr<T>(res);
}

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_REF_COUNT_HPP_
