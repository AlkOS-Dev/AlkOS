#include <extensions/concepts.hpp>
#include <test_module/test.hpp>

class ConceptsTest : public TestGroupBase
{
};

struct A {
};
struct B {
    // B is convertible to A.
    operator A() const { return A{}; }
};

TEST_F(ConceptsTest, ConvertibleToConcept)
{
    // Verify basic type conversions.
    static_assert(std::convertible_to<int, double>, "int is convertible to double");
    static_assert(!std::convertible_to<double, int *>, "double is not convertible to int*");

    // Verify custom type conversion.
    static_assert(std::convertible_to<B, A>, "B is convertible to A");

    // Use a templated lambda to test the concept.
    auto TestLambda = []<typename T>() -> bool {
        if constexpr (std::convertible_to<T, int>)
            return true;
        else
            return false;
    };

    EXPECT_TRUE(TestLambda.operator()<int>());
    EXPECT_FALSE(TestLambda.operator()<double *>());
}

TEST_F(ConceptsTest, RequiresExpressionTest)
{
    // Define a lambda that uses static_cast in a requires-expression.
    auto convertLambda = [](auto val) {
        return static_cast<int>(val);
    };

    // Check that the lambda's return type is convertible to int.
    static_assert(
        std::convertible_to<decltype(convertLambda(3.14)), int>, "Lambda conversion works"
    );
    EXPECT_TRUE((std::convertible_to<decltype(convertLambda(3.14)), int>));
}
