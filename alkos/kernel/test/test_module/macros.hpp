#ifndef ALKOS_KERNEL_TEST_TEST_MODULE_MACROS_HPP_
#define ALKOS_KERNEL_TEST_TEST_MODULE_MACROS_HPP_

#include <defines.hpp>
#include <new.hpp>
#include <test_module/test_group_base.hpp>
#include <test_module/test_module.hpp>

/* must be present here, we cannot use '::' operator because we use fixture name directly in the
 * class name */
using test::AddManualTest;
using test::AddTest;
using test::g_expectFail;
using test::TestGroupBase;

/* Ensure "Uniqueness" for names */
#define ___TEST_TEST_CLASS_NAME(fixture, test_name) \
    ___test_test_obj_##fixture##_##test_name##_74645678651

/* Ensure "Uniqueness" for names */
#define ___TEST_TEST_FACTORY_NAME(fixture, test_name) \
    ___test_test_obj_##fixture##_##test_name##_74645678651_factory

#define ___TEST_TEST_NAME(fixture, test_name) fixture##_##test_name

#define ___TEST_F(fixture, test_name, expect_fail, add_func)                             \
    class ___TEST_TEST_CLASS_NAME(fixture, test_name) final : public fixture             \
    {                                                                                    \
        public:                                                                          \
        ___TEST_TEST_CLASS_NAME(fixture, test_name)()           = default;               \
        ~___TEST_TEST_CLASS_NAME(fixture, test_name)() override = default;               \
        void SetExpectFail_() override { g_expectFail = expect_fail; }                   \
        void Run_() override;                                                            \
                                                                                         \
        private:                                                                         \
        static const bool registered_;                                                   \
    };                                                                                   \
                                                                                         \
    static TestGroupBase* ___TEST_TEST_FACTORY_NAME(fixture, test_name)(void* mem)       \
    {                                                                                    \
        return new (mem) ___TEST_TEST_CLASS_NAME(fixture, test_name)();                  \
    }                                                                                    \
                                                                                         \
    const bool ___TEST_TEST_CLASS_NAME(fixture, test_name)::registered_ = []() -> bool { \
        add_func(                                                                        \
            TOSTRING(___TEST_TEST_NAME(fixture, test_name)),                             \
            ___TEST_TEST_FACTORY_NAME(fixture, test_name)                                \
        );                                                                               \
        return true;                                                                     \
    }();                                                                                 \
                                                                                         \
    void ___TEST_TEST_CLASS_NAME(fixture, test_name)::Run_()

// ------------------------------------
// User macros - automatic tests
// ------------------------------------

#define TEST(test_name) ___TEST_F(TestGroupBase, test_name, false, AddTest)

#define FAIL_TEST(test_name) ___TEST_F(TestGroupBase, test_name, true, AddTest)

#define TEST_F(fixture, test_name) ___TEST_F(fixture, test_name, false, AddTest)

#define FAIL_TEST_F(fixture, test_name) ___TEST_F(fixture, test_name, true, AddTest)

// --------------------------------
// User macros - manual tests
// --------------------------------

#define MTEST(test_name) ___TEST_F(TestGroupBase, test_name, false, AddManualTest)

#define FAIL_MTEST(test_name) ___TEST_F(TestGroupBase, test_name, true, AddManualTest)

#define MTEST_F(fixture, test_name) ___TEST_F(fixture, test_name, false, AddManualTest)

#define FAIL_MTEST_F(fixture, test_name) ___TEST_F(fixture, test_name, true, AddManualTest)

#endif  // ALKOS_KERNEL_TEST_TEST_MODULE_MACROS_HPP_
