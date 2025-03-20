#include <extensions/internal/intervals.hpp>
#include <test_module/test.hpp>

class IntervalsTest : public TestGroupBase
{
};

// === Closed intervals tests using DoIntervalsOverlap ===

TEST_F(IntervalsTest, DoIntervalsOverlap_WhenIntervalsOverlap_ShouldReturnTrue)
{
    // Given
    int start1 = 1, end1 = 5;
    int start2 = 4, end2 = 7;
    // When
    bool result = internal::DoIntervalsOverlap(start1, end1, start2, end2);
    // Then
    ASSERT_TRUE(result);
}

TEST_F(IntervalsTest, DoIntervalsOverlap_WhenIntervalsTouchAtBoundary_ShouldReturnTrue)
{
    // Given
    int start1 = 1, end1 = 5;
    int start2 = 5, end2 = 10;
    // When
    bool result = internal::DoIntervalsOverlap(start1, end1, start2, end2);
    // Then
    ASSERT_TRUE(result);
}

TEST_F(IntervalsTest, DoIntervalsOverlap_WhenIntervalsDoNotOverlap_ShouldReturnFalse)
{
    // Given
    int start1 = 1, end1 = 2;
    int start2 = 3, end2 = 4;
    // When
    bool result = internal::DoIntervalsOverlap(start1, end1, start2, end2);
    // Then
    ASSERT_FALSE(result);
}

TEST_F(IntervalsTest, DoIntervalsOverlap_WhenIntervalsAreIdentical_ShouldReturnTrue)
{
    // Both intervals are the same.
    int start = 3, end = 8;
    bool result = internal::DoIntervalsOverlap(start, end, start, end);
    ASSERT_TRUE(result);
}

TEST_F(IntervalsTest, DoIntervalsOverlap_WhenFirstIntervalIsContainedWithinSecond_ShouldReturnTrue)
{
    // First interval is completely inside the second.
    int start1 = 3, end1 = 7;
    int start2 = 1, end2 = 10;
    bool result = internal::DoIntervalsOverlap(start1, end1, start2, end2);
    ASSERT_TRUE(result);
}

TEST_F(IntervalsTest, DoIntervalsOverlap_WhenSecondIntervalIsContainedWithinFirst_ShouldReturnTrue)
{
    // Second interval is completely inside the first.
    int start1 = 1, end1 = 10;
    int start2 = 4, end2 = 8;
    bool result = internal::DoIntervalsOverlap(start1, end1, start2, end2);
    ASSERT_TRUE(result);
}

TEST_F(IntervalsTest, DoIntervalsOverlap_WhenIntervalsOverlapReversedOrder_ShouldReturnTrue)
{
    // Overlapping intervals with the later interval passed first.
    int start1 = 5, end1 = 10;
    int start2 = 1, end2 = 6;
    bool result = internal::DoIntervalsOverlap(start1, end1, start2, end2);
    ASSERT_TRUE(result);
}

TEST_F(IntervalsTest, DoIntervalsOverlap_WithNegativeBoundariesAndOverlap_ShouldReturnTrue)
{
    // Overlap with negative values.
    int start1 = -10, end1 = -5;
    int start2 = -7, end2 = 0;
    bool result = internal::DoIntervalsOverlap(start1, end1, start2, end2);
    ASSERT_TRUE(result);
}

TEST_F(IntervalsTest, DoIntervalsOverlap_WithNegativeBoundariesAndNoOverlap_ShouldReturnFalse)
{
    // No overlap with negative values.
    int start1 = -10, end1 = -8;
    int start2 = -7, end2 = -5;
    bool result = internal::DoIntervalsOverlap(start1, end1, start2, end2);
    ASSERT_FALSE(result);
}

// === Open intervals tests using DoOpenIntervalsOverlap ===

TEST_F(IntervalsTest, DoOpenIntervalsOverlap_WhenIntervalsOverlap_ShouldReturnTrue)
{
    // Given
    int start1 = 1, end1 = 5;
    int start2 = 4, end2 = 7;
    // When
    bool result = internal::DoOpenIntervalsOverlap(start1, end1, start2, end2);
    // Then
    ASSERT_TRUE(result);
}

TEST_F(IntervalsTest, DoOpenIntervalsOverlap_WhenIntervalsTouchAtBoundary_ShouldReturnFalse)
{
    // Given: Touching boundaries (no interior point shared)
    int start1 = 1, end1 = 5;
    int start2 = 5, end2 = 10;
    // When
    bool result = internal::DoOpenIntervalsOverlap(start1, end1, start2, end2);
    // Then
    ASSERT_FALSE(result);
}

TEST_F(IntervalsTest, DoOpenIntervalsOverlap_WhenIntervalsDoNotOverlap_ShouldReturnFalse)
{
    // Given: Completely separate intervals.
    int start1 = 1, end1 = 2;
    int start2 = 3, end2 = 4;
    // When
    bool result = internal::DoOpenIntervalsOverlap(start1, end1, start2, end2);
    // Then
    ASSERT_FALSE(result);
}

TEST_F(IntervalsTest, DoOpenIntervalsOverlap_WhenIntervalsAreIdentical_ShouldReturnTrue)
{
    // Both open intervals are identical.
    int start = 3, end = 8;
    bool result = internal::DoOpenIntervalsOverlap(start, end, start, end);
    ASSERT_TRUE(result);
}

TEST_F(
    IntervalsTest, DoOpenIntervalsOverlap_WhenFirstIntervalIsContainedWithinSecond_ShouldReturnTrue
)
{
    // First open interval is entirely inside the second.
    int start1 = 3, end1 = 7;
    int start2 = 1, end2 = 10;
    bool result = internal::DoOpenIntervalsOverlap(start1, end1, start2, end2);
    ASSERT_TRUE(result);
}

TEST_F(
    IntervalsTest, DoOpenIntervalsOverlap_WhenSecondIntervalIsContainedWithinFirst_ShouldReturnTrue
)
{
    // Second open interval is entirely inside the first.
    int start1 = 1, end1 = 10;
    int start2 = 4, end2 = 8;
    bool result = internal::DoOpenIntervalsOverlap(start1, end1, start2, end2);
    ASSERT_TRUE(result);
}

TEST_F(IntervalsTest, DoOpenIntervalsOverlap_WhenIntervalsOverlapReversedOrder_ShouldReturnTrue)
{
    // Overlapping intervals in reversed parameter order.
    int start1 = 5, end1 = 10;
    int start2 = 1, end2 = 6;
    bool result = internal::DoOpenIntervalsOverlap(start1, end1, start2, end2);
    ASSERT_TRUE(result);
}

TEST_F(IntervalsTest, DoOpenIntervalsOverlap_WithNegativeBoundariesAndOverlap_ShouldReturnTrue)
{
    // Overlap with negative values in open intervals.
    int start1 = -10, end1 = -5;
    int start2 = -7, end2 = 0;
    bool result = internal::DoOpenIntervalsOverlap(start1, end1, start2, end2);
    ASSERT_TRUE(result);
}

TEST_F(IntervalsTest, DoOpenIntervalsOverlap_WithNegativeBoundariesAndNoOverlap_ShouldReturnFalse)
{
    // Negative values with no overlap.
    int start1 = -10, end1 = -8;
    int start2 = -7, end2 = -5;
    bool result = internal::DoOpenIntervalsOverlap(start1, end1, start2, end2);
    ASSERT_FALSE(result);
}
