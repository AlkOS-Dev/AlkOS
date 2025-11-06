#include <assert.h>
#include <time.h>
#include <extensions/time.hpp>

// ------------------------------
// implementations
// ------------------------------

[[deprecated]] char *asctime(const tm *time_ptr)
{
    static constexpr size_t kSize = 32;
    static char buf[kSize];

    asctime_s(buf, kSize, time_ptr);

    return buf;
}

errno_t asctime_s(char *buf, const rsize_t bufsz, const tm *time_ptr)
{
    static constexpr size_t kMinialSize = 26;

    if (time_ptr == nullptr || bufsz < kMinialSize) {
        if (buf != nullptr && bufsz > 0) {
            buf[0] = '\0';
        }
        return EOVERFLOW;
    }

    if (buf == nullptr) {
        return EOVERFLOW;
    }

    if (!ValidateTm(*time_ptr)) {
        if (buf != nullptr && bufsz > 0) {
            buf[0] = '\0';
        }
        return EINVAL;
    }

    strftime(buf, bufsz, "%a %b %d %H:%M:%S %C%y", time_ptr);

    return NO_ERROR;
}
