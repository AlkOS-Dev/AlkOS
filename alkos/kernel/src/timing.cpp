#include <time.h>
#include <timers.hpp>
#include <timing.hpp>

time_t g_SystemTime = 0;

void InitTiming() { g_SystemTime = QuerySystemTime(); }
