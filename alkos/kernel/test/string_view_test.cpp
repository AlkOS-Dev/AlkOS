#include <extensions/basic_string_view.hpp>
#include <extensions/literals.hpp>
#include <test_module/test.hpp>

using namespace std;

class StringViewTest : public TestGroupBase
{
};

TEST_F(StringViewTest, Constructors)
{
    constexpr string_view sv1;
    R_ASSERT_EQ(0UL, sv1.size());
    R_ASSERT_EQ(nullptr, sv1.data());

    const auto str = "Hello, World!";
    const string_view sv2(str);
    R_ASSERT_EQ(13UL, sv2.size());
    R_ASSERT_EQ(str, sv2.data());

    const string_view sv3(str, 5);
    R_ASSERT_EQ(5UL, sv3.size());
    R_ASSERT_EQ(str, sv3.data());
}

TEST_F(StringViewTest, Substr)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    const auto sub_sv = sv.substr(7, 5);
    R_ASSERT_EQ(5UL, sub_sv.size());
    R_ASSERT_EQ(str + 7, sub_sv.data());
    R_ASSERT_EQ(0, sub_sv.compare("World"));

    const auto empty_sv = sv.substr(0, 0);
    R_ASSERT_EQ(0UL, empty_sv.size());
    R_ASSERT_EQ(str, empty_sv.data());
}

TEST_F(StringViewTest, Find)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    R_ASSERT_EQ(7UL, sv.find('W'));
    R_ASSERT_EQ(7UL, sv.find("World"));
    R_ASSERT_EQ(string_view::npos, sv.find('x'));
    R_ASSERT_EQ(string_view::npos, sv.find("xyz"));
}

TEST_F(StringViewTest, RFind)
{
    using namespace std::literals::string_view_literals;

    R_ASSERT_EQ(6UL, "AB AB AB"sv.rfind("AB"));
    R_ASSERT_EQ(6UL, "AB AB AB"sv.rfind("ABCD", string_view::npos, 2));
    R_ASSERT_EQ(3UL, "AB AB AB"sv.rfind("AB", 5));
    R_ASSERT_EQ(0UL, "AB CD EF"sv.rfind("AB", 0));
    R_ASSERT_EQ(2UL, "B AB AB "sv.rfind("AB", 2));
    R_ASSERT_EQ(string_view::npos, "B AB AB "sv.rfind("AB", 1));
    R_ASSERT_EQ(5UL, "B AB AB "sv.rfind('A'));
    R_ASSERT_EQ(4UL, "AB AB AB"sv.rfind('B', 4));
    R_ASSERT_EQ(string_view::npos, "AB AB AB"sv.rfind('C'));

    const string_view sv("Hello, World!");
    R_ASSERT_EQ(0UL, sv.rfind("Hello"));
    R_ASSERT_EQ(7UL, sv.rfind("World"));
    R_ASSERT_EQ(7UL, sv.rfind("Wo"));
    R_ASSERT_EQ(1UL, sv.rfind("el"));
    R_ASSERT_EQ(12UL, sv.rfind('!'));
    R_ASSERT_EQ(string_view::npos, sv.rfind('z'));
}

TEST_F(StringViewTest, Compare)
{
    const auto str1 = "Hello, World!";
    const auto str2 = "Hello, World!";
    const auto str3 = "Goodbye, World!";
    const string_view sv1(str1);
    const string_view sv2(str2);
    const string_view sv3(str3);

    R_ASSERT_EQ(0, sv1.compare(sv2));
    R_ASSERT_EQ(1, sv1.compare(sv3));
    R_ASSERT_EQ(-1, sv3.compare(sv1));
}

TEST_F(StringViewTest, FindFirstOf)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    R_ASSERT_EQ(7UL, sv.find_first_of('W'));
    R_ASSERT_EQ(2UL, sv.find_first_of("World"));
    R_ASSERT_EQ(string_view::npos, sv.find_first_of('x'));
    R_ASSERT_EQ(string_view::npos, sv.find_first_of("xyz"));
}

TEST_F(StringViewTest, FindLastOf)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    R_ASSERT_EQ(8UL, sv.find_last_of('o'));
    R_ASSERT_EQ(11UL, sv.find_last_of("World"));
    R_ASSERT_EQ(string_view::npos, sv.find_last_of('x'));
    R_ASSERT_EQ(string_view::npos, sv.find_last_of("xyz"));
}

TEST_F(StringViewTest, FindFirstNotOf)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    R_ASSERT_EQ(1UL, sv.find_first_not_of('H'));
    R_ASSERT_EQ(5UL, sv.find_first_not_of("Hello"));
    R_ASSERT_EQ(0UL, sv.find_first_not_of('x'));
}

TEST_F(StringViewTest, FindLastNotOf)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    R_ASSERT_EQ(11UL, sv.find_last_not_of('!'));
    R_ASSERT_EQ(6UL, sv.find_last_not_of("World!"));
    R_ASSERT_EQ(12UL, sv.find_last_not_of('x'));
}

TEST_F(StringViewTest, Empty)
{
    constexpr string_view sv1;
    R_ASSERT_TRUE(sv1.empty());

    const auto str = "Hello";
    const string_view sv2(str);
    R_ASSERT_FALSE(sv2.empty());
}

TEST_F(StringViewTest, Size)
{
    constexpr string_view sv1;
    R_ASSERT_EQ(0UL, sv1.size());

    const auto str = "Hello, World!";
    const string_view sv2(str);
    R_ASSERT_EQ(13UL, sv2.size());

    const string_view sv3(str, 5);
    R_ASSERT_EQ(5UL, sv3.size());
}

TEST_F(StringViewTest, FrontAndBack)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    R_ASSERT_EQ('H', sv.front());
    R_ASSERT_EQ('!', sv.back());
}

TEST_F(StringViewTest, StartsAndEnds)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    R_ASSERT_TRUE(sv.starts_with("Hello"));
    R_ASSERT_FALSE(sv.starts_with("World"));
    R_ASSERT_TRUE(sv.ends_with("World!"));
    R_ASSERT_FALSE(sv.ends_with("Hello"));

    R_ASSERT_TRUE(sv.starts_with('H'));
    R_ASSERT_FALSE(sv.starts_with('W'));
    R_ASSERT_TRUE(sv.ends_with('!'));
    R_ASSERT_FALSE(sv.ends_with('d'));
}

TEST_F(StringViewTest, Swap)
{
    const auto str1 = "Hello";
    const auto str2 = "World!";

    string_view sv1(str1);
    string_view sv2(str2);

    sv1.swap(sv2);
    R_ASSERT_EQ(str2, sv1.data());
    R_ASSERT_EQ(6UL, sv1.size());
    R_ASSERT_EQ(str1, sv2.data());
    R_ASSERT_EQ(5UL, sv2.size());
}

TEST_F(StringViewTest, RemovePrefixSuffix)
{
    const auto str = "Hello, World!";
    string_view sv(str);

    sv.remove_prefix(7);
    R_ASSERT_EQ(str + 7, sv.data());
    R_ASSERT_EQ(6UL, sv.size());
    R_ASSERT_EQ(0, sv.compare("World!"));

    sv.remove_suffix(1);
    R_ASSERT_EQ(5UL, sv.size());
    R_ASSERT_EQ(0, sv.compare("World"));
}

TEST_F(StringViewTest, FindFirstOfEmpty)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    R_ASSERT_EQ(string_view::npos, sv.find_first_of(string_view()));
    R_ASSERT_EQ(0UL, sv.find_first_of('H'));
    R_ASSERT_EQ(string_view::npos, sv.find_first_of('x'));
}

TEST_F(StringViewTest, FindLastOfEmpty)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    R_ASSERT_EQ(string_view::npos, sv.find_last_of(string_view()));
    R_ASSERT_EQ(12UL, sv.find_last_of('!'));
    R_ASSERT_EQ(string_view::npos, sv.find_last_of('x'));
}

TEST_F(StringViewTest, FindFirstNotOfEmpty)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    R_ASSERT_EQ(0UL, sv.find_first_not_of(string_view()));
    R_ASSERT_EQ(1UL, sv.find_first_not_of('H'));
    R_ASSERT_EQ(0UL, sv.find_first_not_of('x'));
}

TEST_F(StringViewTest, FindLastNotOfEmpty)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    R_ASSERT_EQ(12UL, sv.find_last_not_of(string_view()));
    R_ASSERT_EQ(11UL, sv.find_last_not_of('!'));
    R_ASSERT_EQ(12UL, sv.find_last_not_of('x'));
}

TEST_F(StringViewTest, Copy)
{
    const auto str = "Hello, World!";
    const string_view sv(str);

    char buffer[20];
    const size_t copied = sv.copy(buffer, 5);
    buffer[copied]      = '\0';

    R_ASSERT_EQ(5UL, copied);
    R_ASSERT_EQ(0, strcmp(buffer, "Hello"));
}

TEST_F(StringViewTest, Assignment)
{
    const auto str1 = "Hello, World!";
    const auto str2 = "Goodbye, World!";
    const string_view sv1(str1);
    string_view sv2;

    sv2 = sv1;
    R_ASSERT_EQ(str1, sv2.data());
    R_ASSERT_EQ(13UL, sv2.size());

    sv2 = string_view(str2);
    R_ASSERT_EQ(str2, sv2.data());
    R_ASSERT_EQ(15UL, sv2.size());
}

TEST_F(StringViewTest, Operators)
{
    const auto str1 = "Hello, World!";
    const auto str2 = "Hello, World!";
    const auto str3 = "Goodbye, World!";
    string_view sv1(str1);
    string_view sv2(str2);
    string_view sv3(str3);

    R_ASSERT_TRUE(sv1 == sv2);
    R_ASSERT_FALSE(sv1 == sv3);
    R_ASSERT_FALSE(sv2 == sv3);

    R_ASSERT_TRUE(str1 == sv1);
    R_ASSERT_TRUE(str2 == sv2);
    R_ASSERT_TRUE(str3 == sv3);

    R_ASSERT_FALSE(sv1 != sv2);
    R_ASSERT_TRUE(sv1 != sv3);
    R_ASSERT_TRUE(sv2 != sv3);

    R_ASSERT_FALSE(sv1 < sv2);
    R_ASSERT_FALSE(sv1 < sv3);
    R_ASSERT_FALSE(sv2 < sv3);

    R_ASSERT_FALSE(sv1 > sv2);
    R_ASSERT_TRUE(sv1 > sv3);
    R_ASSERT_TRUE(sv2 > sv3);

    R_ASSERT_TRUE(sv1 <= sv2);
    R_ASSERT_FALSE(sv1 <= sv3);
    R_ASSERT_FALSE(sv2 <= sv3);

    R_ASSERT_TRUE(sv1 >= sv2);
    R_ASSERT_TRUE(sv1 >= sv3);
    R_ASSERT_TRUE(sv2 >= sv3);
}

TEST_F(StringViewTest, SwapFunction)
{
    const auto str1 = "Hello";
    const auto str2 = "World";
    string_view sv1(str1);
    string_view sv2(str2);

    swap(sv1, sv2);
    R_ASSERT_EQ(str2, sv1.data());
    R_ASSERT_EQ(5UL, sv1.size());
    R_ASSERT_EQ(str1, sv2.data());
    R_ASSERT_EQ(5UL, sv2.size());
}

TEST_F(StringViewTest, Literals)
{
    using namespace std::literals::string_view_literals;

    auto sv = "Hello, World!"sv;
    R_ASSERT_EQ(13UL, sv.size());
    R_ASSERT_EQ("Hello, World!", sv.data());
}
