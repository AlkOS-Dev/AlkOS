#include <extensions/internal/paths.hpp>
#include <test_module/test.hpp>

class PathsTest : public TestGroupBase
{
};

TEST_F(PathsTest, TransformString_BasicPaths)
{
    auto result1 = TRANSFORM_STRING(path::weakly_canonical, "/home/user");
    EXPECT_STREQ("/home/user", result1.data());

    auto result2 = TRANSFORM_STRING(path::weakly_canonical, "home/user");
    EXPECT_STREQ("home/user", result2.data());

    auto result3 = TRANSFORM_STRING(path::weakly_canonical, "/home/./user");
    EXPECT_STREQ("/home/user", result3.data());

    auto result4 = TRANSFORM_STRING(path::weakly_canonical, "/home/docs/../user");
    EXPECT_STREQ("/home/user", result4.data());

    auto result5 = TRANSFORM_STRING(path::weakly_canonical, "/home//user///docs");
    EXPECT_STREQ("/home/user/docs", result5.data());

    auto result6 = TRANSFORM_STRING(path::weakly_canonical, "/");
    EXPECT_STREQ("/", result6.data());
}

TEST_F(PathsTest, TransformString_EmptyAndSpecialCases)
{
    auto result1 = TRANSFORM_STRING(path::weakly_canonical, "");
    EXPECT_STREQ("", result1.data());

    auto result2 = TRANSFORM_STRING(path::weakly_canonical, ".");
    EXPECT_STREQ(".", result2.data());

    auto result3 = TRANSFORM_STRING(path::weakly_canonical, "..");
    EXPECT_STREQ("..", result3.data());

    auto result4 = TRANSFORM_STRING(path::weakly_canonical, "///");
    EXPECT_STREQ("/", result4.data());
}

TEST_F(PathsTest, TransformString_ComplexPaths)
{
    auto result1 = TRANSFORM_STRING(path::weakly_canonical, "/home/user/../../root/");
    EXPECT_STREQ("/root/", result1.data());

    auto result2 = TRANSFORM_STRING(path::weakly_canonical, "docs/../user/./files");
    EXPECT_STREQ("user/files", result2.data());

    auto result3 = TRANSFORM_STRING(path::weakly_canonical, "/../../test");
    EXPECT_STREQ("/test", result3.data());

    auto result4 = TRANSFORM_STRING(path::weakly_canonical, "user/../../../test");
    EXPECT_STREQ("../../test", result4.data());

    auto result5 =
        TRANSFORM_STRING(path::weakly_canonical, "/home/./user/../docs/./files/../archive");
    EXPECT_STREQ("/home/docs/archive", result5.data());

    auto result6 = TRANSFORM_STRING(path::weakly_canonical, "/home/user/..");
    EXPECT_STREQ("/home/", result6.data());
}

TEST_F(PathsTest, Relative_BasicRelationships)
{
    auto result1 = RELATIVE("/home/user", "/home/user/docs");
    EXPECT_STREQ("docs", result1.data());

    auto result2 = RELATIVE("/home", "/home/user/docs/files");
    EXPECT_STREQ("user/docs/files", result2.data());

    auto result3 = RELATIVE("/home/user/docs", "/home/user");
    EXPECT_STREQ("..", result3.data());

    auto result4 = RELATIVE("/home/user/docs/files", "/home");
    EXPECT_STREQ("../../..", result4.data());
}

TEST_F(PathsTest, Relative_SamePaths)
{
    auto result1 = RELATIVE("/home/user", "/home/user");
    EXPECT_STREQ(".", result1.data());

    auto result2 = RELATIVE("home/user", "home/user");
    EXPECT_STREQ(".", result2.data());

    auto result3 = RELATIVE("/home/./user", "/home/user");
    EXPECT_STREQ(".", result3.data());

    auto result4 = RELATIVE("/home/user/", "/home/user");
    EXPECT_STREQ(".", result4.data());
}

TEST_F(PathsTest, Relative_SiblingAndCousin)
{
    auto result1 = RELATIVE("/home/user/docs", "/home/user/pictures");
    EXPECT_STREQ("../pictures", result1.data());

    auto result2 = RELATIVE("/home/user1/docs", "/home/user2/pictures");
    EXPECT_STREQ("../../user2/pictures", result2.data());

    auto result3 = RELATIVE("/var/log/apache", "/var/www/html");
    EXPECT_STREQ("../../www/html", result3.data());
}

TEST_F(PathsTest, Relative_NonCanonical)
{
    auto result1 = RELATIVE("/home/user/../admin", "/home/user/docs");
    EXPECT_STREQ("../user/docs", result1.data());

    auto result2 = RELATIVE("/home/user", "/home/admin/../user/docs");
    EXPECT_STREQ("docs", result2.data());

    auto result3 = RELATIVE("/home/./user/../admin/.", "/home/user/./docs");
    EXPECT_STREQ("../user/docs", result3.data());

    auto result4 = RELATIVE("/home/user/docs/../..", "/home/admin/./files/../logs");
    EXPECT_STREQ("admin/logs", result4.data());
}

TEST_F(PathsTest, Relative_RelativePaths)
{
    auto result1 = RELATIVE("user/docs", "user/pictures");
    EXPECT_STREQ("../pictures", result1.data());

    auto result2 = RELATIVE("user", "user/docs/files");
    EXPECT_STREQ("docs/files", result2.data());

    auto result3 = RELATIVE("user/docs/files", "user");
    EXPECT_STREQ("../..", result3.data());

    auto result4 = RELATIVE("user/../admin", "user/docs");
    EXPECT_STREQ("../user/docs", result4.data());

    auto result5 = RELATIVE("project/src", "config/settings");
    EXPECT_STREQ("../../config/settings", result5.data());
}

TEST_F(PathsTest, Relative_MixedAbsoluteRelative)
{
    auto result1 = RELATIVE("/home/user", "docs");
    EXPECT_STREQ("", result1.data());

    auto result2 = RELATIVE("home/user", "/docs");
    EXPECT_STREQ("", result2.data());
}

TEST_F(PathsTest, Relative_EdgeCases)
{
    auto result1 = RELATIVE("/", "/");
    EXPECT_STREQ(".", result1.data());

    auto result2 = RELATIVE("/", "/home");
    EXPECT_STREQ("home", result2.data());

    auto result3 = RELATIVE("/home", "/");
    EXPECT_STREQ("..", result3.data());

    auto result4 = RELATIVE("/home/user/docs", "/");
    EXPECT_STREQ("../../..", result4.data());

    auto result5 = RELATIVE("/home/user/", "/home/user/docs/");
    EXPECT_STREQ("docs", result5.data());

    auto result6 = RELATIVE("/a/b", "/a/c");
    EXPECT_STREQ("../c", result6.data());

    auto result7 = RELATIVE("/home//user", "/home/user/docs");
    EXPECT_STREQ("docs", result7.data());
}

TEST_F(PathsTest, Relative_StressTests)
{
    auto result1 = RELATIVE("/a/b/c/d/e/f/g", "/a/b/c/x/y/z");
    EXPECT_STREQ("../../../../x/y/z", result1.data());

    auto result2 =
        RELATIVE("/very/long/directory/name/structure", "/very/long/different/directory/structure");
    EXPECT_STREQ("../../../different/directory/structure", result2.data());

    auto result3 = RELATIVE("../../../x", "a/b/c/d/e");
    EXPECT_STREQ("", result3.data());

    auto result4 = RELATIVE("a/b/c/d/e", "../../../x");
    EXPECT_STREQ("../../../../../../../../x", result4.data());
}
