#include <limits.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <test_module/test.hpp>

class SnprintfTest : public TestGroupBase
{
    protected:
    static constexpr size_t kBufSize = 256;
    char buffer[kBufSize + 1]{};

    void Setup_() override
    {
        memset(buffer, 'X', kBufSize);
        buffer[kBufSize] = '\0';
    }

    [[nodiscard]] bool IsBufferClean(const unsigned long n) const
    {
        for (size_t i = n; i < kBufSize; i++) {
            if (buffer[i] != 'X') {
                return false;
            }
        }
        return true;
    }

    template <typename... Args>
    void VerifyOutput(const char *format, const char *expected, Args... args)
    {
        Setup_();
        const int ret = snprintf(buffer, kBufSize, format, args...);

        EXPECT_STREQ(expected, buffer);
        EXPECT_NEQ(-1, ret);
        EXPECT_EQ(strlen(expected), static_cast<size_t>(ret));
        EXPECT_TRUE(IsBufferClean(strlen(expected) + 1));
    }
};

// ------------------------------
// Basic functionality tests
// ------------------------------

TEST_F(SnprintfTest, BasicFormatting)
{
    // Basic string formatting
    int ret = snprintf(buffer, kBufSize, "Hello, %s!", "world");
    EXPECT_EQ(13, ret);
    EXPECT_STREQ("Hello, world!", buffer);
    EXPECT_TRUE(IsBufferClean(14));

    // Integer formatting
    Setup_();
    ret = snprintf(buffer, kBufSize, "%d", 12345);
    EXPECT_EQ(5, ret);
    EXPECT_STREQ("12345", buffer);
    EXPECT_TRUE(IsBufferClean(6));

    // Multiple arguments
    Setup_();
    ret = snprintf(buffer, kBufSize, "%s %d %.2f", "Test", 42, 3.14159);
    EXPECT_EQ(12, ret);
    EXPECT_STREQ("Test 42 3.14", buffer);
    EXPECT_TRUE(IsBufferClean(14));
}

// ------------------------------
// Buffer size handling tests
// ------------------------------

TEST_F(SnprintfTest, BufferSizeHandling)
{
    // Zero-size buffer
    int ret = snprintf(buffer, 0, "Test");
    EXPECT_EQ(4, ret);  // Should return length needed
    EXPECT_TRUE(IsBufferClean(0));

    // Buffer exactly fits (including null terminator)
    Setup_();
    ret = snprintf(buffer, 5, "Test");
    EXPECT_EQ(4, ret);
    EXPECT_STREQ("Test", buffer);
    EXPECT_TRUE(IsBufferClean(5));

    // Buffer too small
    Setup_();
    ret = snprintf(buffer, 3, "Test");
    EXPECT_EQ(4, ret);
    EXPECT_EQ('T', buffer[0]);
    EXPECT_EQ('e', buffer[1]);
    EXPECT_EQ('\0', buffer[2]);
    EXPECT_TRUE(IsBufferClean(3));
}

// ------------------------------
// Edge case tests
// ------------------------------

TEST_F(SnprintfTest, EdgeCases)
{
    // Empty format string
    int ret = snprintf(buffer, kBufSize, "");
    EXPECT_EQ(0, ret);
    EXPECT_EQ('\0', buffer[0]);
    EXPECT_TRUE(IsBufferClean(1));

    // Test return value for null buffer (ISO C99)
    Setup_();
    ret = snprintf(nullptr, 0, "Test string %d", 42);
    EXPECT_EQ(14, ret);  // Should return number of chars that would be written

    // Test handling of size_t max (POSIX)
    Setup_();
    ret = snprintf(buffer, SIZE_MAX, "Test");
    EXPECT_EQ(4, ret);
    EXPECT_STREQ("Test", buffer);

    Setup_();
    ret = snprintf(buffer, kBufSize, "%.2f", 3.14159);
    EXPECT_EQ(4, ret);
    EXPECT_STREQ("3.14", buffer);

    Setup_();
    ret = snprintf(buffer, kBufSize, "%d", INT_MIN);
    EXPECT_LT(0, ret);
    EXPECT_EQ('-', buffer[0]);

    // Only format specifiers
    Setup_();
    ret = snprintf(buffer, kBufSize, "%%");
    EXPECT_EQ(1, ret);
    EXPECT_STREQ("%", buffer);

    // Long string of format specifiers
    Setup_();
    ret = snprintf(buffer, kBufSize, "%%%d%%%s%%%c", 42, "test", 'X');
    EXPECT_LT(0, ret);
    EXPECT_STREQ("%42%test%X", buffer);

    // Invalid format specifier (implementation-defined behavior)
    Setup_();
    ret = snprintf(buffer, kBufSize, "%y");
    EXPECT_LT(0, ret);  // Should not crash
}

// ------------------------------
// Format Specifier Combinations
// ------------------------------

TEST_F(SnprintfTest, IntegerFormats)
{
    // Basic integer formats
    VerifyOutput("%d", "42", 42);
    VerifyOutput("%i", "42", 42);
    VerifyOutput("%u", "42", 42u);
    VerifyOutput("%o", "52", 42);
    VerifyOutput("%x", "2a", 42);
    VerifyOutput("%X", "2A", 42);

    // Width specifications
    VerifyOutput("%5d", "   42", 42);
    VerifyOutput("%-5d", "42   ", 42);
    VerifyOutput("%05d", "00042", 42);

    // Combinations with signs
    VerifyOutput("%+d", "+42", 42);
    VerifyOutput("% d", " 42", 42);
    VerifyOutput("%+d", "-42", -42);
}

TEST_F(SnprintfTest, FloatFormats)
{
    // Basic float formats
    VerifyOutput("%f", "3.141590", 3.14159);
    VerifyOutput("%F", "3.141590", 3.14159);
    VerifyOutput("%e", "3.141590e+00", 3.14159);
    VerifyOutput("%E", "3.141590E+00", 3.14159);
    VerifyOutput("%g", "3.14159", 3.14159);
    VerifyOutput("%G", "3.14159", 3.14159);

    // Precision specifications
    VerifyOutput("%.2f", "3.14", 3.14159);
    VerifyOutput("%8.2f", "    3.14", 3.14159);
    VerifyOutput("%-8.2f", "3.14    ", 3.14159);

    // Special values
    // TODO
    TODO_BY_THE_END_OF_MILESTONE0
}

TEST_F(SnprintfTest, StringFormats)
{
    // Basic string formats
    VerifyOutput("%s", "hello", "hello");
    VerifyOutput("%10s", "     hello", "hello");
    VerifyOutput("%-10s", "hello     ", "hello");

    // Precision with strings
    VerifyOutput("%.3s", "hel", "hello");
    VerifyOutput("%10.3s", "       hel", "hello");
    VerifyOutput("%-10.3s", "hel       ", "hello");
}

TEST_F(SnprintfTest, CharFormats)
{
    // Basic char formats
    VerifyOutput("%c", "A", 'A');
    VerifyOutput("%3c", "  A", 'A');
    VerifyOutput("%-3c", "A  ", 'A');
}

TEST_F(SnprintfTest, LengthModifiers)
{
    // Long int
    VerifyOutput("%ld", "42", 42L);
    VerifyOutput("%li", "42", 42L);

    // Long long int
    VerifyOutput("%lld", "42", 42LL);
    VerifyOutput("%lli", "42", 42LL);

    // Short int
    VerifyOutput("%hd", "42", static_cast<short>(42));
    VerifyOutput("%hi", "42", static_cast<short>(42));

    // Size_t
    VerifyOutput("%zu", "42", 42_s);
    VerifyOutput("%zd", "42", 42_s);

    // Ptrdiff_t
    VerifyOutput("%td", "42", static_cast<ptrdiff_t>(42));

    // Intmax_t
    VerifyOutput("%jd", "42", static_cast<intmax_t>(42));
}

TEST_F(SnprintfTest, ComplexCombinations)
{
    // Multiple mixed specifiers
    VerifyOutput(
        "Int: %d, Float: %.2f, String: %s", "Int: 42, Float: 3.14, String: hello", 42, 3.14159,
        "hello"
    );

    // Width and precision combinations
    VerifyOutput("%+8.2f:%04d:%-10s", "  +33.14:0042:test      ", 33.14159, 42, "test");

    // Multiple similar types
    VerifyOutput("%d %i %u %x %X", "42 42 42 2a 2A", 42, 42, 42u, 42, 42);

    // Length modifier combinations
    VerifyOutput("%ld %lld %hd %zd", "42 42 42 42", 42L, 42LL, static_cast<short>(42), 42_s);
}

TODO_BY_THE_END_OF_MILESTONE0
// TEST_F(SnprintfTest, PositionalParameters) {
//     // Basic positional parameters
//     VerifyOutput("%2$d %1$d", "2 1", 1, 2);
//     VerifyOutput("%3$s %1$d %2$.2f",
//                  "three 1 2.00",
//                  1, 2.0, "three");
//
//     // Reuse of positional parameters
//     VerifyOutput("%1$d %1$d %1$d",
//                  "42 42 42",
//                  42);
// }

TEST_F(SnprintfTest, FlagCombinations)
{
    // Combining multiple flags
    VerifyOutput("%+05d", "+0042", 42);
    VerifyOutput("% 05d", " 0042", 42);
    VerifyOutput("%+-5d", "+42  ", 42);
    VerifyOutput("% -5d", " 42  ", 42);

    // Width, precision, and flags
    VerifyOutput("%+8.2f", "   +3.14", 3.14159);
    VerifyOutput("%-+8.2f", "+3.14   ", 3.14159);
    VerifyOutput("%08.2f", "00003.14", 3.14159);
}

TEST_F(SnprintfTest, AlternateForm)
{
    // # flag with different formats
    VerifyOutput("%#x", "0x2a", 42);
    VerifyOutput("%#X", "0X2A", 42);
    VerifyOutput("%#o", "052", 42);
    VerifyOutput("%#g", "3.14159", 3.14159);
    VerifyOutput("%#.0f", "42.", 42.0);

    // Combining alternate form with other flags
    VerifyOutput("%#8x", "    0x2a", 42);
    VerifyOutput("%#08x", "0x00002a", 42);
    VerifyOutput("%-#8x", "0x2a    ", 42);
}
