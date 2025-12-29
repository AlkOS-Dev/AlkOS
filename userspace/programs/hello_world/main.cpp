#include <platform.h>
#include <stdio.h>
#include <sys/calls.h>

template <typename... Args>
void printf(const char *format, Args... args)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), format, args...);
    __platform_write_console(buffer);
}

extern "C" int main()
{
    __platform_write_console(
        "\n----------------------------------\n"
        "Hello from User Space via Syscall!\n"
        "----------------------------------\n"
    );

    printf("\nFormatting Test: %d + %d = %d\n\n", 2, 2, 4);

    Timezone tz;
    __platform_get_timezone(&tz);
    printf("Timezone offset (minutes): %d\n\n", tz.west_offset_minutes);

    u64 ticks = __platform_get_clock_ticks_in_second(kTimeUtc);
    printf("Ticks per second (UTC): %llu\n\n", ticks);

    TimeVal tv;
    __platform_get_clock_value(kTimeUtc, &tv, &tz);
    printf("Current Timestamp: %llu\n\n", tv.seconds);

    return 0;
}
