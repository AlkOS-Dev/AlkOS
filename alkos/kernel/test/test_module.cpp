/* internal includes */
#include <assert.h>
#include <string.h>
#include <arch_utils.hpp>
#include <terminal.hpp>
#include <test_module/test_module.hpp>

namespace test
{
// --------------------------------
// Various defines and usings
// --------------------------------

/* Whole test framework should be reworked when threads are available */
TODO_WHEN_MULTITHREADING

/* 64 MB memory for tests framework - used because of lack of vmem */
TODO_WHEN_VMEM_WORKS
static constexpr u32 kStubMemSize = 1 * 1024 * 1024;

/* size of test array  */
static constexpr u32 kMaxTests = 4096;

/* size of manual test array */
static constexpr u32 kMaxManualTests = 256;

// ------------------------------------
// Definition of global variables
// ------------------------------------

bool g_expectFail{};
bool g_testCheckFailed{};
bool g_testStarted{};

// ------------------------------------
// Definition of static variables
// ------------------------------------

static char g_testMem[kStubMemSize]{};
static TestSpec g_tests[kMaxTests]{};
static u32 g_numTests{};
static TestSpec g_manualTests[kMaxManualTests]{};
static u32 g_numManualTests{};

// ------------------------------
// Method implementations
// ------------------------------

void TestModule::RunTestModule()
{
    static constexpr size_t kInputBufferSize = 512;

    /* display all possible tests */
    DisplayTests_();

    /* read single line of input */
    char buff[kInputBufferSize];
    TerminalWriteString("Provide test name for framework or simply type \"exit\" to quit...\n");
    if (TerminalReadLine(buff, kInputBufferSize) == kInputBufferSize) {
        TerminalWriteError("[TEST] [FAIL] Too long input message...\n");
        QemuShutdown();
    }

    if (strcmp(buff, "exit") == 0) {
        TerminalWriteString("Kernel shutdown on test...\n");
        QemuShutdown();
    }

    const TestSpec *test = FindTestFunction(buff);

    if (test == nullptr) {
        TerminalWriteError("[TEST] [FAIL] Test not found...\n");
        QemuShutdown();
    }

    RunTest_(test);
    QemuShutdown();
}

void TestModule::DisplayTests_()
{
    TerminalWriteString("Displaying list of all tests:\n");
    for (size_t idx = 0; idx < g_numTests; ++idx) {
        TerminalWriteString("[TEST] [TESTNAME] ");
        TerminalWriteString(g_tests[idx].name);
        TerminalWriteString("\n");
    }

    TerminalWriteString("Displaying list of all manual tests:\n");
    for (size_t idx = 0; idx < g_numManualTests; ++idx) {
        TerminalWriteString("[TEST] [MANUAL] [TESTNAME] ");
        TerminalWriteString(g_manualTests[idx].name);
        TerminalWriteString("\n");
    }

    TerminalWriteString("[TEST] [LISTEND]\n");
}

TestSpec *TestModule::FindTestFunction(const char *name)
{
    /* Look for automatic tests, as they are more frequently used */
    for (size_t idx = 0; idx < g_numTests; ++idx) {
        if (strcmp(name, g_tests[idx].name) == 0) {
            return g_tests + idx;
        }
    }

    /* finally check manual tests */
    for (size_t idx = 0; idx < g_numManualTests; ++idx) {
        if (strcmp(name, g_manualTests[idx].name) == 0) {
            return g_manualTests + idx;
        }
    }

    return nullptr;
}

void TestModule::RunTest_(const TestSpec *test)
{
    TerminalWriteString("Running test:\n");
    TerminalWriteString(test->name);
    TerminalWriteString("\n");

    TestGroupBase *test_obj = test->factory(g_testMem);
    R_ASSERT_NOT_NULL(test_obj);

    /* mark that the test is already started */
    g_testStarted = true;
    test_obj->Run();
    delete test_obj;

    if (g_expectFail && !g_testCheckFailed) {
        /* test was expected to fail */
        TerminalWriteError(
            "[TEST] [FAIL] Test was supposed to fail but all checks passed correctly...\n"
        );
        return;
    }

    if (g_testCheckFailed) {
        TerminalWriteError("[TEST] [FAIL] Test failed on some EXPECT_* checks...\n");
        return;
    }

    TerminalWriteString("[TEST] [SUCCESS] Test passed...\n");
}

void AddTest(const char *name, const test_factory_t factory)
{
    R_ASSERT_NULL(TestModule::FindTestFunction(name));
    TestSpec *pTestSpec = &g_tests[g_numTests++];

    pTestSpec->factory = factory;
    pTestSpec->name    = name;
}

void AddManualTest(const char *name, const test_factory_t factory)
{
    R_ASSERT_NULL(TestModule::FindTestFunction(name));
    TestSpec *pTestSpec = &g_manualTests[g_numManualTests++];

    pTestSpec->factory = factory;
    pTestSpec->name    = name;
}

NO_RET void OnKernelPanic()
{
    if (!g_testStarted) {
        /* test not started yet, but we received failure -> some critical bug -> abort execution */
        TerminalWriteError("[TEST] [FAIL] Kernel panic received before test started...\n");
        QemuShutdown();
    }

    if (g_expectFail) {
        /* fail was expected we are good */
        TerminalWriteString("[TEST] [SUCCESS] Test failed successfully...\n");
        QemuShutdown();
    }

    /* ups... */
    TerminalWriteError("[TEST] [FAIL] Test failed on kernel panic...\n");
    QemuShutdown();
}
}  // namespace test
