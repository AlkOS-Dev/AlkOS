#include <extensions/internal/paths.hpp>
#include <test_module/test.hpp>

class PathsTest : public TestGroupBase
{
};

TEST_F(PathsTest, TransformString_BasicPaths)
{
    auto result1 = TRANSFORM_STRING(path::WeaklyCanonical, "/home/user");
    EXPECT_STREQ("/home/user", result1.data());

    auto result2 = TRANSFORM_STRING(path::WeaklyCanonical, "home/user");
    EXPECT_STREQ("home/user", result2.data());

    auto result3 = TRANSFORM_STRING(path::WeaklyCanonical, "/home/./user");
    EXPECT_STREQ("/home/user", result3.data());

    auto result4 = TRANSFORM_STRING(path::WeaklyCanonical, "/home/docs/../user");
    EXPECT_STREQ("/home/user", result4.data());

    auto result5 = TRANSFORM_STRING(path::WeaklyCanonical, "/home//user///docs");
    EXPECT_STREQ("/home/user/docs", result5.data());

    auto result6 = TRANSFORM_STRING(path::WeaklyCanonical, "/");
    EXPECT_STREQ("/", result6.data());
}

TEST_F(PathsTest, TransformString_EmptyAndSpecialCases)
{
    auto result1 = TRANSFORM_STRING(path::WeaklyCanonical, "");
    EXPECT_STREQ("", result1.data());

    auto result2 = TRANSFORM_STRING(path::WeaklyCanonical, ".");
    EXPECT_STREQ(".", result2.data());

    auto result3 = TRANSFORM_STRING(path::WeaklyCanonical, "..");
    EXPECT_STREQ("..", result3.data());

    auto result4 = TRANSFORM_STRING(path::WeaklyCanonical, "///");
    EXPECT_STREQ("/", result4.data());
}

TEST_F(PathsTest, TransformString_ComplexPaths)
{
    auto result1 = TRANSFORM_STRING(path::WeaklyCanonical, "/home/user/../../root/");
    EXPECT_STREQ("/root/", result1.data());

    auto result2 = TRANSFORM_STRING(path::WeaklyCanonical, "docs/../user/./files");
    EXPECT_STREQ("user/files", result2.data());

    auto result3 = TRANSFORM_STRING(path::WeaklyCanonical, "/../../test");
    EXPECT_STREQ("/test", result3.data());

    auto result4 = TRANSFORM_STRING(path::WeaklyCanonical, "user/../../../test");
    EXPECT_STREQ("../../test", result4.data());

    auto result5 =
        TRANSFORM_STRING(path::WeaklyCanonical, "/home/./user/../docs/./files/../archive");
    EXPECT_STREQ("/home/docs/archive", result5.data());

    auto result6 = TRANSFORM_STRING(path::WeaklyCanonical, "/home/user/..");
    EXPECT_STREQ("/home/", result6.data());
}

TEST_F(PathsTest, RelativeDir_BasicRelationships)
{
    auto result1 = RELATIVE_DIR("/home/user", "/home/user/docs");
    EXPECT_STREQ("docs", result1.data());

    auto result2 = RELATIVE_DIR("/home", "/home/user/docs/files");
    EXPECT_STREQ("user/docs/files", result2.data());

    auto result3 = RELATIVE_DIR("/home/user/docs", "/home/user");
    EXPECT_STREQ("..", result3.data());

    auto result4 = RELATIVE_DIR("/home/user/docs/files", "/home");
    EXPECT_STREQ("../../..", result4.data());
}

TEST_F(PathsTest, RelativeDir_SamePaths)
{
    auto result1 = RELATIVE_DIR("/home/user", "/home/user");
    EXPECT_STREQ(".", result1.data());

    auto result2 = RELATIVE_DIR("home/user", "home/user");
    EXPECT_STREQ(".", result2.data());

    auto result3 = RELATIVE_DIR("/home/./user", "/home/user");
    EXPECT_STREQ(".", result3.data());

    auto result4 = RELATIVE_DIR("/home/user/", "/home/user");
    EXPECT_STREQ(".", result4.data());
}

TEST_F(PathsTest, RelativeDir_SiblingAndCousin)
{
    auto result1 = RELATIVE_DIR("/home/user/docs", "/home/user/pictures");
    EXPECT_STREQ("../pictures", result1.data());

    auto result2 = RELATIVE_DIR("/home/user1/docs", "/home/user2/pictures");
    EXPECT_STREQ("../../user2/pictures", result2.data());

    auto result3 = RELATIVE_DIR("/var/log/apache", "/var/www/html");
    EXPECT_STREQ("../../www/html", result3.data());
}

TEST_F(PathsTest, RelativeDir_NonCanonical)
{
    auto result1 = RELATIVE_DIR("/home/user/../admin", "/home/user/docs");
    EXPECT_STREQ("../user/docs", result1.data());

    auto result2 = RELATIVE_DIR("/home/user", "/home/admin/../user/docs");
    EXPECT_STREQ("docs", result2.data());

    auto result3 = RELATIVE_DIR("/home/./user/../admin/.", "/home/user/./docs");
    EXPECT_STREQ("../user/docs", result3.data());

    auto result4 = RELATIVE_DIR("/home/user/docs/../..", "/home/admin/./files/../logs");
    EXPECT_STREQ("admin/logs", result4.data());
}

TEST_F(PathsTest, RelativeDir_RelativePaths)
{
    auto result1 = RELATIVE_DIR("user/docs", "user/pictures");
    EXPECT_STREQ("../pictures", result1.data());

    auto result2 = RELATIVE_DIR("user", "user/docs/files");
    EXPECT_STREQ("docs/files", result2.data());

    auto result3 = RELATIVE_DIR("user/docs/files", "user");
    EXPECT_STREQ("../..", result3.data());

    auto result4 = RELATIVE_DIR("user/../admin", "user/docs");
    EXPECT_STREQ("../user/docs", result4.data());

    auto result5 = RELATIVE_DIR("project/src", "config/settings");
    EXPECT_STREQ("../../config/settings", result5.data());
}

TEST_F(PathsTest, RelativeDir_MixedAbsoluteRelative)
{
    auto result1 = RELATIVE_DIR("/home/user", "docs");
    EXPECT_STREQ("", result1.data());

    auto result2 = RELATIVE_DIR("home/user", "/docs");
    EXPECT_STREQ("", result2.data());
}

TEST_F(PathsTest, RelativeDir_EdgeCases)
{
    auto result1 = RELATIVE_DIR("/", "/");
    EXPECT_STREQ(".", result1.data());

    auto result2 = RELATIVE_DIR("/", "/home");
    EXPECT_STREQ("home", result2.data());

    auto result3 = RELATIVE_DIR("/home", "/");
    EXPECT_STREQ("..", result3.data());

    auto result4 = RELATIVE_DIR("/home/user/docs", "/");
    EXPECT_STREQ("../../..", result4.data());

    auto result5 = RELATIVE_DIR("/home/user/", "/home/user/docs/");
    EXPECT_STREQ("docs", result5.data());

    auto result6 = RELATIVE_DIR("/a/b", "/a/c");
    EXPECT_STREQ("../c", result6.data());

    auto result7 = RELATIVE_DIR("/home//user", "/home/user/docs");
    EXPECT_STREQ("docs", result7.data());
}

TEST_F(PathsTest, RelativeDir_StressTests)
{
    auto result1 = RELATIVE_DIR("/a/b/c/d/e/f/g", "/a/b/c/x/y/z");
    EXPECT_STREQ("../../../../x/y/z", result1.data());

    auto result2 = RELATIVE_DIR(
        "/very/long/directory/name/structure", "/very/long/different/directory/structure"
    );
    EXPECT_STREQ("../../../different/directory/structure", result2.data());

    auto result3 = RELATIVE_DIR("../../../x", "a/b/c/d/e");
    EXPECT_STREQ("", result3.data());

    auto result4 = RELATIVE_DIR("a/b/c/d/e", "../../../x");
    EXPECT_STREQ("../../../../../../../../x", result4.data());
}

TEST_F(PathsTest, Relative_BasicRelationships)
{
    auto result1 = RELATIVE("/home/user", "/home/user/docs/report.pdf");
    EXPECT_STREQ("docs/report.pdf", result1.data());

    auto result2 = RELATIVE("/home", "/home/user/docs/files/data.txt");
    EXPECT_STREQ("user/docs/files/data.txt", result2.data());

    auto result3 = RELATIVE("/home/user/docs", "/home/user/project.cpp");
    EXPECT_STREQ("../project.cpp", result3.data());

    auto result4 = RELATIVE("/home/user/docs/files", "/home/index.html");
    EXPECT_STREQ("../../../index.html", result4.data());
}

TEST_F(PathsTest, Relative_SamePaths)
{
    auto result1 = RELATIVE("/home/user", "/home/user/file.txt");
    EXPECT_STREQ("file.txt", result1.data());

    auto result2 = RELATIVE("home/user", "home/user/file.txt");
    EXPECT_STREQ("file.txt", result2.data());

    auto result3 = RELATIVE("/home/./user", "/home/user/file.txt");
    EXPECT_STREQ("file.txt", result3.data());

    auto result4 = RELATIVE("/home/user/", "/home/user/file.txt");
    EXPECT_STREQ("file.txt", result4.data());
}

TEST_F(PathsTest, Relative_SiblingAndCousin)
{
    auto result1 = RELATIVE("/home/user/docs", "/home/user/pictures/vacation.jpg");
    EXPECT_STREQ("../pictures/vacation.jpg", result1.data());

    auto result2 = RELATIVE("/home/user1/docs", "/home/user2/pictures/photo.png");
    EXPECT_STREQ("../../user2/pictures/photo.png", result2.data());

    auto result3 = RELATIVE("/var/log/apache", "/var/www/html/index.html");
    EXPECT_STREQ("../../www/html/index.html", result3.data());
}

TEST_F(PathsTest, Relative_NonCanonical)
{
    auto result1 = RELATIVE("/home/user/../admin", "/home/user/docs/report.pdf");
    EXPECT_STREQ("../user/docs/report.pdf", result1.data());

    auto result2 = RELATIVE("/home/user", "/home/admin/../user/docs/report.pdf");
    EXPECT_STREQ("docs/report.pdf", result2.data());

    auto result3 = RELATIVE("/home/./user/../admin/.", "/home/user/./docs/file.txt");
    EXPECT_STREQ("../user/docs/file.txt", result3.data());

    auto result4 = RELATIVE("/home/user/docs/../..", "/home/admin/./files/../logs/system.log");
    EXPECT_STREQ("admin/logs/system.log", result4.data());
}

TEST_F(PathsTest, Relative_RelativePaths)
{
    auto result1 = RELATIVE("user/docs", "user/pictures/vacation.jpg");
    EXPECT_STREQ("../pictures/vacation.jpg", result1.data());

    auto result2 = RELATIVE("user", "user/docs/files/document.pdf");
    EXPECT_STREQ("docs/files/document.pdf", result2.data());

    auto result3 = RELATIVE("user/docs/files", "user/main.cpp");
    EXPECT_STREQ("../../main.cpp", result3.data());

    auto result4 = RELATIVE("user/../admin", "user/docs/report.pdf");
    EXPECT_STREQ("../user/docs/report.pdf", result4.data());

    auto result5 = RELATIVE("project/src", "config/settings/config.ini");
    EXPECT_STREQ("../../config/settings/config.ini", result5.data());
}

TEST_F(PathsTest, Relative_MixedAbsoluteRelative)
{
    auto result1 = RELATIVE("/home/user", "docs/report.pdf");
    EXPECT_STREQ("", result1.data());

    auto result2 = RELATIVE("home/user", "/docs/report.pdf");
    EXPECT_STREQ("", result2.data());
}

TEST_F(PathsTest, Relative_EdgeCases)
{
    auto result1 = RELATIVE("/", "/index.html");
    EXPECT_STREQ("index.html", result1.data());

    auto result2 = RELATIVE("/", "/home/user/file.txt");
    EXPECT_STREQ("home/user/file.txt", result2.data());

    auto result3 = RELATIVE("/home", "/readme.md");
    EXPECT_STREQ("../readme.md", result3.data());

    auto result4 = RELATIVE("/home/user/docs", "/index.html");
    EXPECT_STREQ("../../../index.html", result4.data());

    auto result5 = RELATIVE("/home/user/", "/home/user/docs/file.txt");
    EXPECT_STREQ("docs/file.txt", result5.data());

    auto result6 = RELATIVE("/a/b", "/a/file.txt");
    EXPECT_STREQ("../file.txt", result6.data());

    auto result7 = RELATIVE("/home//user", "/home/user/docs/report.pdf");
    EXPECT_STREQ("docs/report.pdf", result7.data());
}

TEST_F(PathsTest, Relative_StressTests)
{
    auto result1 = RELATIVE("/a/b/c/d/e/f/g", "/a/b/c/x/y/z/file.bin");
    EXPECT_STREQ("../../../../x/y/z/file.bin", result1.data());

    auto result2 = RELATIVE(
        "/very/long/directory/name/structure", "/very/long/different/directory/structure/file.ext"
    );
    EXPECT_STREQ("../../../different/directory/structure/file.ext", result2.data());

    auto result3 = RELATIVE("../../../x", "a/b/c/d/e/file.txt");
    EXPECT_STREQ("", result3.data());

    auto result4 = RELATIVE("a/b/c/d/e", "../../../x/file.dat");
    EXPECT_STREQ("../../../../../../../../x/file.dat", result4.data());
}
