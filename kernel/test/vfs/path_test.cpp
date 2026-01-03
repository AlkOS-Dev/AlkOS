#include <test_module/test.hpp>

#include <fs/vfs/path.hpp>

class VFSPathTest : public TestGroupBase
{
};

// =============================================================================
// Construction and Basic Properties
// =============================================================================

TEST_F(VFSPathTest, DefaultAndEmptyPaths)
{
    // Default constructor
    vfs::Path empty_default;
    EXPECT_TRUE(empty_default.IsEmpty());
    EXPECT_FALSE(empty_default.IsAbsolute());
    EXPECT_TRUE(empty_default.IsRelative());
    EXPECT_FALSE(empty_default.IsRoot());
    EXPECT_EQ(0u, empty_default.ComponentCount());

    // Empty string
    vfs::Path empty_string("");
    EXPECT_TRUE(empty_string.IsEmpty());
    EXPECT_EQ(0u, empty_string.ComponentCount());

    // Only slashes (should be treated as root)
    vfs::Path only_slashes("///");
    EXPECT_TRUE(only_slashes.IsAbsolute());
    EXPECT_TRUE(only_slashes.IsRoot());
    EXPECT_EQ(0u, only_slashes.ComponentCount());
}

TEST_F(VFSPathTest, RootPath)
{
    vfs::Path path("/");
    EXPECT_FALSE(path.IsEmpty());
    EXPECT_TRUE(path.IsAbsolute());
    EXPECT_FALSE(path.IsRelative());
    EXPECT_TRUE(path.IsRoot());
    EXPECT_EQ(0u, path.ComponentCount());
    EXPECT_TRUE(path.GetFilename().empty());
}

TEST_F(VFSPathTest, AbsolutePaths)
{
    // Single component
    vfs::Path single("/home");
    EXPECT_FALSE(single.IsEmpty());
    EXPECT_TRUE(single.IsAbsolute());
    EXPECT_FALSE(single.IsRoot());
    EXPECT_EQ(1u, single.ComponentCount());
    EXPECT_EQ("home", single.GetComponent(0));

    // Multiple components
    vfs::Path multi("/home/user/documents");
    EXPECT_TRUE(multi.IsAbsolute());
    EXPECT_EQ(3u, multi.ComponentCount());
    EXPECT_EQ("home", multi.GetComponent(0));
    EXPECT_EQ("user", multi.GetComponent(1));
    EXPECT_EQ("documents", multi.GetComponent(2));

    // Trailing slash (should be ignored)
    vfs::Path trailing("/home/user/");
    EXPECT_TRUE(trailing.IsAbsolute());
    EXPECT_EQ(2u, trailing.ComponentCount());
    EXPECT_EQ("home", trailing.GetComponent(0));
    EXPECT_EQ("user", trailing.GetComponent(1));

    // Multiple consecutive slashes (should be treated as single)
    vfs::Path multi_slash("/home//user///documents");
    EXPECT_TRUE(multi_slash.IsAbsolute());
    EXPECT_EQ(3u, multi_slash.ComponentCount());
    EXPECT_EQ("home", multi_slash.GetComponent(0));
    EXPECT_EQ("user", multi_slash.GetComponent(1));
    EXPECT_EQ("documents", multi_slash.GetComponent(2));
}

TEST_F(VFSPathTest, RelativePaths)
{
    // Single component
    vfs::Path single("file.txt");
    EXPECT_FALSE(single.IsEmpty());
    EXPECT_FALSE(single.IsAbsolute());
    EXPECT_TRUE(single.IsRelative());
    EXPECT_EQ(1u, single.ComponentCount());
    EXPECT_EQ("file.txt", single.GetComponent(0));

    // Multiple components
    vfs::Path multi("dir/subdir/file.txt");
    EXPECT_TRUE(multi.IsRelative());
    EXPECT_EQ(3u, multi.ComponentCount());
    EXPECT_EQ("dir", multi.GetComponent(0));
    EXPECT_EQ("subdir", multi.GetComponent(1));
    EXPECT_EQ("file.txt", multi.GetComponent(2));
}

// =============================================================================
// Filename, Stem, and Extension
// =============================================================================

TEST_F(VFSPathTest, FilenameExtraction)
{
    // Simple filename with extension
    EXPECT_EQ("file.txt", vfs::Path("/home/user/file.txt").GetFilename());

    // Filename without extension
    EXPECT_EQ("README", vfs::Path("/home/user/README").GetFilename());

    // Root has no filename
    EXPECT_TRUE(vfs::Path("/").GetFilename().empty());

    // Empty path has no filename
    EXPECT_TRUE(vfs::Path().GetFilename().empty());
}

TEST_F(VFSPathTest, StemExtraction)
{
    // Simple case
    EXPECT_EQ("file", vfs::Path("/home/user/file.txt").GetStem());

    // Multiple dots - stem is everything before last dot
    EXPECT_EQ("file.tar", vfs::Path("/home/user/file.tar.gz").GetStem());

    // No extension
    EXPECT_EQ("README", vfs::Path("/home/user/README").GetStem());

    // Dot file (leading dot) - entire name is stem
    EXPECT_EQ(".bashrc", vfs::Path("/home/user/.bashrc").GetStem());
}

TEST_F(VFSPathTest, ExtensionExtraction)
{
    // Simple extension
    EXPECT_EQ(".txt", vfs::Path("/home/user/file.txt").GetExtension());

    // Multiple dots - only last extension
    EXPECT_EQ(".gz", vfs::Path("/home/user/file.tar.gz").GetExtension());

    // No extension
    EXPECT_TRUE(vfs::Path("/home/user/README").GetExtension().empty());

    // Dot file has no extension
    EXPECT_TRUE(vfs::Path("/home/user/.bashrc").GetExtension().empty());
}

// =============================================================================
// Parent Path
// =============================================================================

TEST_F(VFSPathTest, ParentPathNavigation)
{
    // Absolute path parent
    vfs::Path abs_path("/home/user/file.txt");
    vfs::Path abs_parent = abs_path.GetParent();
    EXPECT_TRUE(abs_parent.IsAbsolute());
    EXPECT_EQ(2u, abs_parent.ComponentCount());
    EXPECT_EQ("home", abs_parent.GetComponent(0));
    EXPECT_EQ("user", abs_parent.GetComponent(1));

    // Root's parent is still root
    vfs::Path root("/");
    EXPECT_TRUE(root.GetParent().IsRoot());

    // Single component absolute path's parent is root
    vfs::Path single_abs("/home");
    EXPECT_TRUE(single_abs.GetParent().IsRoot());

    // Relative path parent
    vfs::Path rel_path("dir/file.txt");
    vfs::Path rel_parent = rel_path.GetParent();
    EXPECT_TRUE(rel_parent.IsRelative());
    EXPECT_EQ(1u, rel_parent.ComponentCount());
    EXPECT_EQ("dir", rel_parent.GetComponent(0));

    // Single component relative path's parent is current dir
    vfs::Path single_rel("file.txt");
    EXPECT_EQ(single_rel.GetParent(), vfs::Path::kCurrentDir);
}

// =============================================================================
// Path Concatenation
// =============================================================================

TEST_F(VFSPathTest, PathConcatenation)
{
    // Absolute + relative
    vfs::Path base("/home");
    vfs::Path rel("user");
    vfs::Path result = base / rel;
    EXPECT_TRUE(result.IsAbsolute());
    EXPECT_EQ(2u, result.ComponentCount());
    EXPECT_EQ("home", result.GetComponent(0));
    EXPECT_EQ("user", result.GetComponent(1));

    // Absolute + absolute (second replaces first)
    vfs::Path abs("/etc");
    vfs::Path replaced = base / abs;
    EXPECT_TRUE(replaced.IsAbsolute());
    EXPECT_EQ("etc", replaced.GetComponent(0));

    // Chaining with strings
    vfs::Path chained = vfs::Path("/home") / "user" / "documents";
    EXPECT_TRUE(chained.IsAbsolute());
    EXPECT_EQ(3u, chained.ComponentCount());
    EXPECT_EQ("home", chained.GetComponent(0));
    EXPECT_EQ("user", chained.GetComponent(1));
    EXPECT_EQ("documents", chained.GetComponent(2));

    // Assignment operator
    vfs::Path assigned("/home");
    assigned /= "user";
    assigned /= "documents";
    EXPECT_TRUE(assigned.IsAbsolute());
    EXPECT_EQ(3u, assigned.ComponentCount());
}

// =============================================================================
// Path Normalization
// =============================================================================

TEST_F(VFSPathTest, PathNormalization)
{
    // Remove single dots
    vfs::Path dot_path("/home/./user");
    vfs::Path dot_norm = dot_path.GetNormalized();
    EXPECT_EQ(2u, dot_norm.ComponentCount());
    EXPECT_EQ("home", dot_norm.GetComponent(0));
    EXPECT_EQ("user", dot_norm.GetComponent(1));

    // Resolve double dots
    vfs::Path dotdot_path("/home/user/../documents");
    vfs::Path dotdot_norm = dotdot_path.GetNormalized();
    EXPECT_EQ(2u, dotdot_norm.ComponentCount());
    EXPECT_EQ("home", dotdot_norm.GetComponent(0));
    EXPECT_EQ("documents", dotdot_norm.GetComponent(1));

    // Complex path with mixed . and ..
    vfs::Path complex("/home/user/./documents/../downloads/./file.txt");
    vfs::Path complex_norm = complex.GetNormalized();
    EXPECT_EQ(4u, complex_norm.ComponentCount());
    EXPECT_EQ("home", complex_norm.GetComponent(0));
    EXPECT_EQ("user", complex_norm.GetComponent(1));
    EXPECT_EQ("downloads", complex_norm.GetComponent(2));
    EXPECT_EQ("file.txt", complex_norm.GetComponent(3));

    // Double dot at root is ignored for absolute paths
    vfs::Path dotdot_root("/../home");
    vfs::Path dotdot_root_norm = dotdot_root.GetNormalized();
    EXPECT_TRUE(dotdot_root_norm.IsAbsolute());
    EXPECT_EQ(1u, dotdot_root_norm.ComponentCount());
    EXPECT_EQ("home", dotdot_root_norm.GetComponent(0));

    // Relative path with double dots
    vfs::Path rel_dotdot("a/b/../c");
    vfs::Path rel_dotdot_norm = rel_dotdot.GetNormalized();
    EXPECT_TRUE(rel_dotdot_norm.IsRelative());
    EXPECT_EQ(2u, rel_dotdot_norm.ComponentCount());
    EXPECT_EQ("a", rel_dotdot_norm.GetComponent(0));
    EXPECT_EQ("c", rel_dotdot_norm.GetComponent(1));

    // Relative path going past root keeps the ..
    vfs::Path past_root("a/../..");
    vfs::Path past_root_norm = past_root.GetNormalized();
    EXPECT_TRUE(past_root_norm.IsRelative());
    EXPECT_EQ(1u, past_root_norm.ComponentCount());
    EXPECT_EQ("..", past_root_norm.GetComponent(0));
}

// =============================================================================
// Comparison Operators
// =============================================================================

TEST_F(VFSPathTest, PathComparison)
{
    // Equality
    vfs::Path path1("/home/user");
    vfs::Path path2("/home/user");
    vfs::Path path3("/home/other");
    EXPECT_TRUE(path1 == path2);
    EXPECT_FALSE(path1 != path2);
    EXPECT_FALSE(path1 == path3);
    EXPECT_TRUE(path1 != path3);

    // Absolute vs relative are not equal
    vfs::Path abs("/home");
    vfs::Path rel("home");
    EXPECT_FALSE(abs == rel);

    // Less than ordering
    vfs::Path a("/a/b");
    vfs::Path c("/a/c");
    EXPECT_TRUE(a < c);

    // Relative paths are "less than" absolute
    EXPECT_TRUE(rel < abs);
}

// =============================================================================
// Iteration
// =============================================================================

TEST_F(VFSPathTest, IteratorAndForEach)
{
    vfs::Path path("/home/user/documents");

    // Range-based for loop
    size_t count = 0;
    for (const auto &p : path) {
        (void)p;
        ++count;
    }
    EXPECT_EQ(3u, count);

    // Iterator values
    auto it = path.begin();
    EXPECT_EQ("home", *it);
    ++it;
    EXPECT_EQ("user", *it);
    ++it;
    EXPECT_EQ("documents", *it);
    ++it;
    EXPECT_TRUE(it == path.end());

    // Empty path (root) has no components
    vfs::Path root("/");
    EXPECT_TRUE(root.begin() == root.end());

    // ForEachComponent
    size_t foreach_count = 0;
    path.ForEachComponent([&](std::string_view) {
        ++foreach_count;
    });
    EXPECT_EQ(3u, foreach_count);

    // ForEachComponentReverse
    vfs::Path abc("/a/b/c");
    std::array<std::string_view, 3> expected = {"c", "b", "a"};
    size_t idx                               = 0;
    abc.ForEachComponentReverse([&](std::string_view component) {
        EXPECT_EQ(expected[idx++], component);
    });
    EXPECT_EQ(3u, idx);
}

// =============================================================================
// Static Methods and Constants
// =============================================================================

TEST_F(VFSPathTest, StaticMethodsAndConstants)
{
    // Path::Join
    vfs::Path joined = vfs::Path::Join("/home", "user", "documents");
    EXPECT_TRUE(joined.IsAbsolute());
    EXPECT_EQ(3u, joined.ComponentCount());

    // Static constants
    EXPECT_EQ(1u, vfs::Path::kCurrentDir.ComponentCount());
    EXPECT_EQ(".", vfs::Path::kCurrentDir.GetComponent(0));

    EXPECT_EQ(1u, vfs::Path::kParentDir.ComponentCount());
    EXPECT_EQ("..", vfs::Path::kParentDir.GetComponent(0));

    EXPECT_TRUE(vfs::Path::kRoot.IsRoot());
    EXPECT_TRUE(vfs::Path::kRoot.IsAbsolute());
}

// =============================================================================
// Copy and Move Semantics
// =============================================================================

TEST_F(VFSPathTest, CopyAndMoveSemantics)
{
    vfs::Path original("/home/user/documents");

    // Copy construction
    vfs::Path copy_constructed(original);
    EXPECT_EQ(original, copy_constructed);
    EXPECT_EQ(3u, copy_constructed.ComponentCount());

    // Copy assignment
    vfs::Path copy_assigned;
    copy_assigned = original;
    EXPECT_EQ(original, copy_assigned);

    // Move construction
    vfs::Path to_move1("/home/user/documents");
    vfs::Path move_constructed(std::move(to_move1));
    EXPECT_EQ(3u, move_constructed.ComponentCount());
    EXPECT_EQ("home", move_constructed.GetComponent(0));

    // Move assignment
    vfs::Path to_move2("/home/user/documents");
    vfs::Path move_assigned;
    move_assigned = std::move(to_move2);
    EXPECT_EQ(3u, move_assigned.ComponentCount());
}

// =============================================================================
// String Access and Edge Cases
// =============================================================================

TEST_F(VFSPathTest, StringAccessAndEdgeCases)
{
    vfs::Path path("/home/user");

    // StringView access
    std::string_view sv = path.StringView();
    EXPECT_EQ("/home/user", sv);

    // CString access
    const char *cstr = path.CString();
    EXPECT_STREQ("/home/user", cstr);

    // Out of bounds component access returns empty
    EXPECT_TRUE(path.GetComponent(100).empty());
    EXPECT_TRUE(path[100].empty());
}
