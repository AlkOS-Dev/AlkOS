#include <assert.h>
#include <time.h>

char *ctime(const time_t *timer)
{
    assert(false && "Not implemented!");
    return nullptr;
}

errno_t ctime_s(char *buf, rsize_t bufsz, const time_t *timer)
{
    assert(false && "Not implemented!");
    return errno_t{};
}
