#include <test_module/test.hpp>
#include <type_traits.hpp>
#include <utility.hpp>

using namespace std;

class UtilityTest : public TestGroupBase
{
};

TEST_F(UtilityTest, MakeIndexSequence_WithSizeN_GeneratesSequenceZeroToNMinusOne)
{
    using seq_0 = make_index_sequence<0>;
    using seq_1 = make_index_sequence<1>;
    using seq_5 = make_index_sequence<5>;

    EXPECT_TRUE((is_same_v<seq_0, index_sequence<>>));
    EXPECT_TRUE((is_same_v<seq_1, index_sequence<0>>));
    EXPECT_TRUE((is_same_v<seq_5, index_sequence<0, 1, 2, 3, 4>>));
}

TEST_F(UtilityTest, IntegerSequenceSize_ForGivenSequence_ReturnsCorrectCount)
{
    using seq_0     = integer_sequence<int>;
    using seq_3     = integer_sequence<long, 0, 1, 2>;
    using seq_empty = make_index_sequence<0>;

    static_assert(seq_0::size() == 0);
    static_assert(seq_3::size() == 3);
    static_assert(seq_empty::size() == 0);

    EXPECT_EQ(0, seq_0::size());
    EXPECT_EQ(3, seq_3::size());
}
