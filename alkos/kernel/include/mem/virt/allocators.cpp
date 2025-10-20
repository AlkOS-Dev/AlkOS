#include "mem/allocators.hpp"

mem::Expected<mem::VirtualPtr<void>, mem::MemError> alloca::AlignedKMalloc(
    const size_t size, const size_t alignment
)
{
    const size_t full_size = size + alignment;  // safety buffer
    auto result            = mem::KMalloc(full_size);

    if (!result) {
        return result;
    }

    const u64 addr     = reinterpret_cast<u64>(result.value());
    const u64 reminder = addr % alignment;
    if (reminder == 0) {
        return result.value();
    }

    return reinterpret_cast<void *>(addr + alignment - reminder);
}
