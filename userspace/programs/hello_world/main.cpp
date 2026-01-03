#include <stdio.h>

extern "C" int main()
{
    printf(
        "\n----------------------------------\n"
        "Hello from User Space via Syscall!\n"
        "----------------------------------\n"
    );

    printf("Formatting Test: %d + %d = %d\n", 2, 2, 4);

    char buffer[64]{};
    FILE *fp = fopen("/docs/greet.txt", "r+");
    if (!fp) {
        printf("Failed to open /docs/greet.txt for reading!\n");
        return 1;
    }

    int len        = snprintf(buffer, sizeof(buffer), "Hello AlkOS from User Space!");
    size_t written = fwrite(buffer, 1, len, fp);
    if (written != static_cast<size_t>(len)) {
        printf("Failed to write to /docs/greet.txt!\n");
        fclose(fp);
        return 1;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        printf("Failed to seek to start of /docs/greet.txt!\n");
        fclose(fp);
        return 1;
    }

    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, fp);
    if (bytes_read == 0 && ferror(fp)) {
        printf("Failed to read from /docs/greet.txt!\n");
        fclose(fp);
        return 1;
    }
    buffer[bytes_read] = '\0';

    printf("Read from file: '%s'\n", buffer);

    fclose(fp);

    return 0;
}
