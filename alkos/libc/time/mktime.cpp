#include <assert.h>
#include <errno.h>
#include <sys/calls.h>
#include <time.h>
#include <extensions/time.hpp>

// ------------------------------
// Implementation
// ------------------------------

time_t mktime(tm *time_ptr)
{
    const auto time_zone = GetTimezoneSysCall();
    const time_t t       = ConvertDateTimeToPosix(*time_ptr, time_zone);

    if (t == kConversionFailed) {
        errno = EOVERFLOW;
        return kMktimeFailed;
    }

    /* TODO: use localtime to adjust tm structure */
    localtime_r(&t, time_ptr);

    return t;
}
