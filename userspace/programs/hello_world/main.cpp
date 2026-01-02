#include <alkos/calls.h>
#include <platform.h>
#include <stdio.h>

template <typename... Args>
void my_printf(const char *format, Args... args)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), format, args...);
    __platform_debug_write(buffer);
}

extern "C" int main()
{
    my_printf(
        "\n----------------------------------\n"
        "Hello from User Space via Syscall!\n"
        "----------------------------------\n"
    );

    my_printf("Formatting Test: %d + %d = %d\n", 2, 2, 4);

    Timezone tz;
    __platform_get_timezone(&tz);
    my_printf("Timezone offset (minutes): %d\n", tz.west_offset_minutes);

    u64 ticks = __platform_get_clock_ticks_in_second(kTimeUtc);
    my_printf("Ticks per second (UTC): %llu\n", ticks);

    TimeVal tv;
    __platform_get_clock_value(kTimeUtc, &tv, &tz);
    my_printf("Current Timestamp: %llu\n", tv.seconds);

    char buffer[64]{};
    fd_t fd = __platform_open("/docs/greet.txt", kFdFlagReadWrite);
    if (fd < 3) {
        my_printf("KWorker failed to open /docs/greet.txt for reading!");
    }

    int len         = snprintf(buffer, sizeof(buffer), "Hello AlkOS from User Space!");
    ssize_t written = __platform_write(fd, buffer, len);
    if (written == -1) {
        my_printf("KWorker failed to write to /docs/greet.txt!");
    }

    ssize_t pos = __platform_seek(fd, 0, kFdSeekSet);
    if (pos == -1) {
        my_printf("KWorker failed to seek to start of /docs/greet.txt!");
    }

    ssize_t read = __platform_read(fd, buffer, 64);
    if (read == -1) {
        my_printf("KWorker failed to read from /docs/greet.txt!");
    }

    my_printf("KWorker: '%.*s'", static_cast<int>(written), buffer);

    __platform_close(fd);

    return 0;
}
