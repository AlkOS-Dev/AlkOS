/* internal includes */
#include <time.h>
#include <test_module/test.hpp>

class TimeTests : public TestGroupBase
{
    public:
    /* 07.01.2025 1:31pm */
    static constexpr time_t kTestTime1 = 1737117056;
};

TEST_F(TimeTests, TestTime) { localtime_r(&kTestTime1, nullptr); }
