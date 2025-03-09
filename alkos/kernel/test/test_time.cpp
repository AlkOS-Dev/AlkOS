/* internal includes */
#include <time.h>
#include <test_module/test.hpp>

class TimeTestFixture : public TestGroupBase
{
    public:
    /* 07.01.2025 1:31pm */
    static constexpr time_t kTestTime1 = 1737117056;
};

TEST_F(TimeTestFixture, TestTime) { localtime_r(&kTestTime1, nullptr); }
