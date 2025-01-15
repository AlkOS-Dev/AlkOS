#include <assert.h>
#include <time.h>

[[deprecated]] char *asctime(const tm *time_ptr)
{
    assert(false && "Not implemented!");
    return nullptr;
}

errno_t asctime_s(char *buf, rsize_t bufsz, const tm *time_ptr)
{
    assert(false && "Not implemented!");
    return errno_t{};
}
