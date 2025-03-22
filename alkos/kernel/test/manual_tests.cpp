#include <assert.h>
#include <test_module/test.hpp>

MTEST(AssertSnprintfTest) { ASSERT_EQ(2, 3, "Values used in the test: %d, %d", 2, 3); }
