#include <memory.h>
#include <string.h>
#include <test_module/test.hpp>

class StringTest : public TestGroupBase
{
    protected:
    static constexpr size_t kBufferSize = 256;
    char buffer[kBufferSize]{};
};

// ------------------------------
// strlen tests
// ------------------------------

TEST_F(StringTest, Strlen)
{
    R_ASSERT_EQ(13_size, strlen("Hello, World!"));
    R_ASSERT_ZERO(strlen(""));
}

// ------------------------------
// strcmp tests
// ------------------------------

TEST_F(StringTest, Strcmp)
{
    R_ASSERT_ZERO(strcmp("hello", "hello"));
    R_ASSERT_LT(strcmp("hello", "world"), 0);
    R_ASSERT_GT(strcmp("world", "hello"), 0);
    R_ASSERT_ZERO(strcmp("", ""));
}

// ------------------------------
// strncmp tests
// ------------------------------

TEST_F(StringTest, Strncmp)
{
    R_ASSERT_ZERO(strncmp("hello", "hello", 5));
    R_ASSERT_ZERO(strncmp("hello", "help", 3));
    R_ASSERT_LT(strncmp("hello", "help", 4), 0);
    R_ASSERT_GT(strncmp("help", "hello", 4), 0);
}

// ------------------------------
// strcpy tests
// ------------------------------

TEST_F(StringTest, Strcpy)
{
    const char *src = "Hello";
    R_ASSERT_EQ(buffer, strcpy(buffer, src));
    R_ASSERT_ZERO(strcmp(buffer, src));

    R_ASSERT_EQ(buffer, strcpy(buffer, ""));
    R_ASSERT_ZERO(buffer[0]);
}

// ------------------------------
// strncpy tests
// ------------------------------

TEST_F(StringTest, StrncpyFull)
{
    const char *src = "Hello";
    R_ASSERT_EQ(buffer, strncpy(buffer, src, 6));
    R_ASSERT_ZERO(strcmp(buffer, src));
}

TEST_F(StringTest, StrncpyPartial)
{
    const char *src = "Hello, World!";
    R_ASSERT_EQ(buffer, strncpy(buffer, src, 5));
    R_ASSERT_ZERO(strncmp(buffer, src, 5));
}

TEST_F(StringTest, StrncpyPadding)
{
    const char *src = "Hi";
    memset(buffer, 'X', kBufferSize);
    R_ASSERT_EQ(buffer, strncpy(buffer, src, 5));

    // First two chars should be "Hi"
    R_ASSERT_EQ('H', buffer[0]);
    R_ASSERT_EQ('i', buffer[1]);

    // Next three should be null
    for (size_t i = 2; i < 5; i++) {
        R_ASSERT_ZERO(buffer[i]);
    }
}

// ------------------------------
// strcat tests
// ------------------------------

TEST_F(StringTest, StrcarBasic)
{
    strcpy(buffer, "Hello");
    R_ASSERT_EQ(buffer, strcat(buffer, " World"));
    R_ASSERT_ZERO(strcmp(buffer, "Hello World"));
}

TEST_F(StringTest, StrcatEmpty)
{
    strcpy(buffer, "Hello");
    R_ASSERT_EQ(buffer, strcat(buffer, ""));
    R_ASSERT_ZERO(strcmp(buffer, "Hello"));
}

// ------------------------------
// strncat tests
// ------------------------------

TEST_F(StringTest, StrncatFull)
{
    strcpy(buffer, "Hello");
    R_ASSERT_EQ(buffer, strncat(buffer, " World", 6));
    R_ASSERT_ZERO(strcmp(buffer, "Hello World"));
}

TEST_F(StringTest, StrncatPartial)
{
    strcpy(buffer, "Hello");
    R_ASSERT_EQ(buffer, strncat(buffer, " World", 3));
    R_ASSERT_ZERO(strcmp(buffer, "Hello Wo"));
}

// ------------------------------
// strchr tests
// ------------------------------

TEST_F(StringTest, Strchr)
{
    const char *str = "Hello, World!";
    R_ASSERT_EQ(str + 7, strchr(str, 'W'));

    R_ASSERT_EQ(nullptr, strchr(str, 'Z'));

    const char *str1 = "Hello";
    R_ASSERT_EQ(str1 + 5, strchr(str1, '\0'));
}

// ------------------------------
// strrchr tests
// ------------------------------

TEST_F(StringTest, Strrchr)
{
    const char *str = "Hello, World!";
    R_ASSERT_EQ(str + 12, strrchr(str, '!'));

    R_ASSERT_EQ(nullptr, strrchr(str, 'Z'));

    const char *str1 = "Hello World World";
    R_ASSERT_EQ(str1 + 12, strrchr(str1, 'W'));

    const char *str2 = "Hello";
    R_ASSERT_EQ(str2 + 5, strrchr(str2, '\0'));
}
