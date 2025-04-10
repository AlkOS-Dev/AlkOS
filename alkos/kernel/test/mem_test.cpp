#include <string.h>
#include <test_module/test.hpp>

class MemTest : public TestGroupBase
{
    protected:
    static constexpr size_t kMemChunkSize    = 13;
    static constexpr size_t kBigMemChunkSize = 2 * kMemChunkSize;

    char big_mem_chunk[kBigMemChunkSize]{};
    char mem_chunk[kMemChunkSize] = "ABCD1234ABCD";
};

// ------------------------------
// memcmp tests
// ------------------------------

TEST_F(MemTest, MemcmpEqual)
{
    char str1[] = "ABCD1234";
    char str2[] = "ABCD1234";

    R_ASSERT_ZERO(memcmp(str1, str2, sizeof(str1)));
}

TEST_F(MemTest, MemcmpLess)
{
    char str1[] = "ABCD1234";
    char str2[] = "ABCD1235";

    R_ASSERT_LT(memcmp(str1, str2, sizeof(str1)), 0);
}

TEST_F(MemTest, MemcmpGreater)
{
    char str1[] = "ABCD1235";
    char str2[] = "ABCD1234";

    R_ASSERT_GT(memcmp(str1, str2, sizeof(str1)), 0);
}

TEST_F(MemTest, MemcmpZeroSize)
{
    char str1[] = "ABCD1234";
    char str2[] = "12345678";

    R_ASSERT_ZERO(memcmp(str1, str2, 0));
}

TEST_F(MemTest, MemcmpPartial)
{
    char str1[] = "ABCD1234";
    char str2[] = "ABCD5678";

    R_ASSERT_ZERO(memcmp(str1, str2, 4));
}

// ------------------------------
// memset tests
// ------------------------------

TEST_F(MemTest, MemsetBasic)
{
    R_ASSERT_EQ(big_mem_chunk, static_cast<char *>(memset(big_mem_chunk, 'X', kMemChunkSize)));
    for (size_t i = 0; i < kMemChunkSize; i++) {
        R_ASSERT_EQ('X', big_mem_chunk[i]);
    }

    R_ASSERT_EQ(mem_chunk, static_cast<char *>(memset(mem_chunk, 0, kMemChunkSize)));
    for (const char x : mem_chunk) {
        R_ASSERT_ZERO(x);
    }
}

TEST_F(MemTest, MemsetZeroBytes)
{
    R_ASSERT_EQ(big_mem_chunk, static_cast<char *>(memset(big_mem_chunk, 'X', 0)));

    for (const char i : big_mem_chunk) {
        R_ASSERT_ZERO(i);
    }
}

// ------------------------------
// memove tests
// ------------------------------

TEST_F(MemTest, MemmoveNoOverlap)
{
    static constexpr auto expected = "ABCD1234ABCD";

    R_ASSERT_EQ(
        big_mem_chunk, static_cast<char *>(memmove(big_mem_chunk, mem_chunk, kMemChunkSize))
    );
    R_ASSERT_ZERO(memcmp(big_mem_chunk, expected, kMemChunkSize));
}

TEST_F(MemTest, MemmoveForwardOverlap)
{
    char test_buffer[] = "ABCD1234ABCD";
    R_ASSERT_EQ(test_buffer + 2, static_cast<char *>(memmove(test_buffer + 2, test_buffer, 8)));
    R_ASSERT_ZERO(memcmp(test_buffer + 2, "ABCD1234", 8));
}

TEST_F(MemTest, MemmoveBackwardOverlap)
{
    char test_buffer[] = "ABCD1234ABCD";
    R_ASSERT_EQ(test_buffer, static_cast<char *>(memmove(test_buffer, test_buffer + 2, 8)));
    R_ASSERT_ZERO(memcmp(test_buffer, "CD1234AB", 8));
}

// ------------------------------
// memcpy test
// ------------------------------

TEST_F(MemTest, MemcpyTest)
{
    char expected[] = "ABCD1234ABCD";

    R_ASSERT_EQ(
        big_mem_chunk, static_cast<char *>(memcpy(big_mem_chunk, mem_chunk, kMemChunkSize))
    );
    R_ASSERT_ZERO(memcmp(big_mem_chunk, expected, kMemChunkSize));
}
