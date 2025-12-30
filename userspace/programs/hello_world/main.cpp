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

    char buffer[64]{};
    fd_t fd = __platform_open("/docs/greet.txt", kFdFlagReadWrite);
    if (fd < 0) {
        printf("KWorker failed to open /docs/greet.txt for reading!");
    }

    int len         = snprintf(buffer, sizeof(buffer), "Hello AlkOS from User Space!");
    ssize_t written = __platform_write(fd, buffer, len);
    if (written == -1) {
        printf("KWorker failed to write to /docs/greet.txt!");
    }

    ssize_t pos = __platform_seek(fd, 0, kFdSeekSet);
    if (pos == -1) {
        printf("KWorker failed to seek to start of /docs/greet.txt!");
    }

    ssize_t read = __platform_read(fd, buffer, 64);
    if (read == -1) {
        printf("KWorker failed to read from /docs/greet.txt!");
    }

    printf("KWorker: '%.*s'", static_cast<int>(written), buffer);

    return 0;
}
