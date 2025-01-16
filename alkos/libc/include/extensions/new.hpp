#ifndef LIBC_INCLUDE_NEW_HPP_
#define LIBC_INCLUDE_NEW_HPP_

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
    const size_t offset =
        static_cast<size_t>(al) - (reinterpret_cast<size_t>(ptr) % static_cast<size_t>(al));
    return reinterpret_cast<char *>(ptr) + offset;
}

inline void *operator new[](size_t, std::align_val_t al, void *ptr)
{
    const size_t offset =
        static_cast<size_t>(al) - (reinterpret_cast<size_t>(ptr) % static_cast<size_t>(al));
    return reinterpret_cast<char *>(ptr) + offset;
}

#endif  // LIBC_INCLUDE_NEW_HPP_
