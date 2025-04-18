#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_NEW_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_NEW_HPP_

#include <assert.h>
#include <stddef.h>

namespace std
{
enum class align_val_t : size_t {};
}

// ------------------------------
// In place new definitions
// ------------------------------

inline void *operator new(size_t, void *ptr) noexcept { return ptr; }

inline void *operator new[](size_t, void *ptr) noexcept { return ptr; }

inline void operator delete(void *, void *) noexcept {}

inline void operator delete[](void *, void *) noexcept {}

// --------------------------------------
// In place aligned new definitions
// --------------------------------------

inline void operator delete(void *, size_t, std::align_val_t) noexcept {}

inline void operator delete[](void *, size_t, std::align_val_t) noexcept {}

/* Be careful with aligned allocations as it may need more space than sizeof() */
inline void *operator new(size_t, std::align_val_t al, void *ptr)
{
    ASSERT_EQ(0_size, reinterpret_cast<size_t>(ptr) % static_cast<size_t>(al));
    return ptr;
}

inline void *operator new[](size_t, std::align_val_t al, void *ptr)
{
    ASSERT_EQ(0_size, reinterpret_cast<size_t>(ptr) % static_cast<size_t>(al));
    return ptr;
}

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_NEW_HPP_
