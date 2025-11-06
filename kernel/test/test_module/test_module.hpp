#ifndef KERNEL_TEST_TEST_MODULE_TEST_MODULE_HPP_
#define KERNEL_TEST_TEST_MODULE_TEST_MODULE_HPP_

/* internal includes */
#include <defines.hpp>
#include <test_module/test_group_base.hpp>

namespace test
{

// --------------------------------
// Various defines and usings
// --------------------------------

/* should use given memory to construct test object - used due to lack of vmem */
using test_factory_t = TestGroupBase *(*)(void *);

struct TestSpec {
    const char *name;
    test_factory_t factory;
};

// ------------------------------
// Main Test module class
// ------------------------------

class TestModule final
{
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    TestModule()  = default;
    ~TestModule() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void RunTestModule();
    static TestSpec *FindTestFunction(const char *name);

    // ------------------------------
    // Class private methods
    // ------------------------------

    private:
    static void DisplayTests_();
    static void RunTest_(const TestSpec *test);

    // ------------------------------
    // Class fields
    // ------------------------------
};

// ------------------------------
// Various functions
// ------------------------------

void AddTest(const char *name, test_factory_t factory);

void AddManualTest(const char *name, test_factory_t factory);

NO_RET void OnKernelPanic();

// ------------------------------
// Global objects
// ------------------------------

/* this flag controls whether given test case should fail - in this case fail means total failure on
 * some assert */
extern bool g_expectFail;

/* this flag controls whether during the test execution some EXPECT_* check failed */
extern bool g_testCheckFailed;

/* this flag controls whether current test is already started */
extern bool g_testStarted;

}  // namespace test

#endif  // KERNEL_TEST_TEST_MODULE_TEST_MODULE_HPP_
