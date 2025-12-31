#include <platform.h>
#include <stdio.h>
#include <sys/calls.h>

template <typename... Args>
void printf(const char *format, Args... args)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), format, args...);
    __platform_debug_write(buffer);
}

extern "C" int main()
{
    printf(
        "\n----------------------------------\n"
        "Hello from User Space via Syscall!\n"
        "----------------------------------\n"
    );

    printf("Formatting Test: %d + %d = %d\n", 2, 2, 4);

    Timezone tz;
    __platform_get_timezone(&tz);
    printf("Timezone offset (minutes): %d\n", tz.west_offset_minutes);

    u64 ticks = __platform_get_clock_ticks_in_second(kTimeUtc);
    printf("Ticks per second (UTC): %llu\n", ticks);

    TimeVal tv;
    __platform_get_clock_value(kTimeUtc, &tv, &tz);
    printf("Current Timestamp: %llu\n", tv.seconds);

    while (true) {
        static size_t kSpins = 1'000'000;
        size_t counter       = 0;
        for (size_t i = 0; i < kSpins; i++) {
            ++counter;
        }
    }

    return 0;
}
