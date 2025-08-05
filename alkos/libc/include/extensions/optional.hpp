#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_OPTIONAL_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_OPTIONAL_HPP_

#include <assert.h>
#include <todo.h>
#include <extensions/defines.hpp>
#include <extensions/type_traits.hpp>

namespace std
{

TODO_LIBC_EXT1
// TODO: Partial implementation

template <class T>
class optional
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    optional() = default;

    ~optional() { reset(); }

    // ------------------------------
    // Class interaction
    // ------------------------------

    void reset()
    {
        if (initialized_) {
            T* ptr = reinterpret_cast<T*>(mem_.data);

            ptr->~T();
            initialized_ = false;
        }
    }

    template <typename... Args>
    T& emplace(Args&&... args)
    {
        reset();

        new (reinterpret_cast<void*>(mem_.data)) T(std::forward<Args>(args)...);
        initialized_ = true;

        return *reinterpret_cast<T*>(mem_.data);
    }

    NODISCARD bool has_value() const noexcept { return initialized_; }

    explicit operator bool() const noexcept { return initialized_; }

    T& operator*() noexcept
    {
        ASSERT_TRUE(initialized_);
        return *reinterpret_cast<T*>(mem_.data);
    }

    const T& operator*() const noexcept
    {
        ASSERT_TRUE(initialized_);
        return *reinterpret_cast<T*>(mem_.data);
    }

    T* operator->() noexcept
    {
        ASSERT_TRUE(initialized_);
        return reinterpret_cast<T*>(mem_.data);
    }

    const T* operator->() const noexcept
    {
        ASSERT_TRUE(initialized_);
        return reinterpret_cast<T*>(mem_.data);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    std::aligned_storage_t<sizeof(T), alignof(T)> mem_;
    bool initialized_;
};

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_OPTIONAL_HPP_
