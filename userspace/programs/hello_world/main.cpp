#include <stdio.h>

extern "C" int main()
{
    printf(
        "\n----------------------------------\n"
        "Hello from User Space via Syscall!\n"
        "----------------------------------\n"
    );

    FILE *fp = fopen("/docs/greet.txt", "r+");
    if (!fp) {
        printf("Failed to open /docs/greet.txt for reading!\n");
        return 1;
    }

    int written = fprintf(fp, "Hello AlkOS from User Space!");
    if (written < 0) {
        printf("Failed to write to /docs/greet.txt!\n");
        fclose(fp);
        return 1;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        printf("Failed to seek to start of /docs/greet.txt!\n");
        fclose(fp);
        return 1;
    }

    char buffer[128];
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
