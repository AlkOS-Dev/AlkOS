#include <assert.h>
#include <time.h>

char *ctime(const time_t *timer)
{
    static constexpr size_t kSize = 32;
    static char buf[kSize];

    ctime_s(buf, kSize, timer);

    return buf;
}

errno_t ctime_s(char *buf, const rsize_t bufsz, const time_t *timer)
{
    /* error checking */
    TODO_LIBC_EXT1

    if (timer == nullptr) {
        if (buf != nullptr && bufsz != 0) {
            buf[0] = '\0';
        }
        return EOVERFLOW;
    }

    return asctime_s(buf, bufsz, localtime_r(timer, {}));
}
