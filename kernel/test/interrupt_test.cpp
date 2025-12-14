#include <test_module/test.hpp>

class InterruptTest : public TestGroupBase
{
};

void ExceptionFailsKernelTest();
FAIL_TEST_F(InterruptTest, ExceptionFailsKernelTest) { ExceptionFailsKernelTest(); }

void ExceptionTestSavesAllRegisters();
TEST_F(InterruptTest, ExceptionTestSavesAllRegisters) { ExceptionTestSavesAllRegisters(); }
