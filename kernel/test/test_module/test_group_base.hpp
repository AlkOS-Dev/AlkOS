#ifndef ALKOS_KERNEL_TEST_TEST_MODULE_TEST_GROUP_BASE_HPP_
#define ALKOS_KERNEL_TEST_TEST_MODULE_TEST_GROUP_BASE_HPP_

namespace test
{

class TestGroupBase
{
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    TestGroupBase()          = default;
    virtual ~TestGroupBase() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void Run()
    {
        SetExpectFail_();
        Setup_();
        Run_();
        TearDown_();
    }

    protected:
    /* to be overridden by user */
    virtual void Setup_() {}
    virtual void TearDown_() {}

    /* internal methods to run tests */
    virtual void Run_() {}
    virtual void SetExpectFail_() {}

    // ------------------------------
    // Class fields
    // ------------------------------
};

}  // namespace test

#endif  // ALKOS_KERNEL_TEST_TEST_MODULE_TEST_GROUP_BASE_HPP_
