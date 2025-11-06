#include <sys/calls.h>
#include <time.h>
#include <extensions/time.hpp>

// ------------------------------
// Implementation
// ------------------------------

time_t mktime(tm *time_ptr) { return MkTimeFromTimeZone(*time_ptr, GetTimezoneSysCall()); }
