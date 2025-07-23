#include <extensions/internal/paths.hpp>
#include <test_module/test.hpp>

class PathsTest : public TestGroupBase
{
};

TEST_F(PathsTest, WeaklyCanonicalPath)
{
    auto result = TRANSFORM_STRING(weakly_canonical_path, "/test///abc/.././bca");
    EXPECT_STREQ("/test/bca", result.data());
}
