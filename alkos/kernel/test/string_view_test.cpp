#include <extensions/basic_string_view.hpp>
#include <extensions/literals.hpp>
#include <test_module/test.hpp>

using namespace std;

class StringViewTest : public TestGroupBase
{
};

TEST_F(StringViewTest, BasicStringViewConstructors)
{
    constexpr basic_string_view<char> sv1;
    R_ASSERT_EQ(0, sv1.size());
    R_ASSERT_EQ(nullptr, sv1.data());

    const auto str = "Hello, World!";
    const basic_string_view<char> sv2(str);
    R_ASSERT_EQ(13, sv2.size());
    R_ASSERT_EQ(str, sv2.data());

    const basic_string_view<char> sv3(str, 5);
    R_ASSERT_EQ(5, sv3.size());
    R_ASSERT_EQ(str, sv3.data());
}

TEST_F(StringViewTest, BasicStringViewSubstr)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    const auto sub_sv = sv.substr(7, 5);
    R_ASSERT_EQ(5, sub_sv.size());
    R_ASSERT_EQ(str + 7, sub_sv.data());
    R_ASSERT_EQ(0, sub_sv.compare("World"));

    const auto empty_sv = sv.substr(0, 0);
    R_ASSERT_EQ(0, empty_sv.size());
    R_ASSERT_EQ(str, empty_sv.data());
}

TEST_F(StringViewTest, BasicStringViewFind)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    R_ASSERT_EQ(7, sv.find('W'));
    R_ASSERT_EQ(7, sv.find("World"));
    R_ASSERT_EQ(basic_string_view<char>::npos, sv.find('x'));
    R_ASSERT_EQ(basic_string_view<char>::npos, sv.find("xyz"));
}

TEST_F(StringViewTest, BasicStringViewCompare)
{
    const auto str1 = "Hello, World!";
    const auto str2 = "Hello, World!";
    const auto str3 = "Goodbye, World!";
    const basic_string_view<char> sv1(str1);
    const basic_string_view<char> sv2(str2);
    const basic_string_view<char> sv3(str3);

    R_ASSERT_EQ(0, sv1.compare(sv2));
    R_ASSERT_EQ(1, sv1.compare(sv3));
    R_ASSERT_EQ(-1, sv3.compare(sv1));
}

TEST_F(StringViewTest, BasicStringViewFindFirstOf)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    R_ASSERT_EQ(7, sv.find_first_of('W'));
    R_ASSERT_EQ(2, sv.find_first_of("World"));
    R_ASSERT_EQ(basic_string_view<char>::npos, sv.find_first_of('x'));
    R_ASSERT_EQ(basic_string_view<char>::npos, sv.find_first_of("xyz"));
}

TEST_F(StringViewTest, BasicStringViewFindLastOf)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    R_ASSERT_EQ(8, sv.find_last_of('o'));
    R_ASSERT_EQ(11, sv.find_last_of("World"));
    R_ASSERT_EQ(basic_string_view<char>::npos, sv.find_last_of('x'));
    R_ASSERT_EQ(basic_string_view<char>::npos, sv.find_last_of("xyz"));
}

TEST_F(StringViewTest, BasicStringViewFindFirstNotOf)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    R_ASSERT_EQ(1, sv.find_first_not_of('H'));
    R_ASSERT_EQ(5, sv.find_first_not_of("Hello"));
    R_ASSERT_EQ(0, sv.find_first_not_of('x'));
}

TEST_F(StringViewTest, BasicStringViewFindLastNotOf)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    R_ASSERT_EQ(11, sv.find_last_not_of('!'));
    R_ASSERT_EQ(6, sv.find_last_not_of("World!"));
    R_ASSERT_EQ(12, sv.find_last_not_of('x'));
}

TEST_F(StringViewTest, BasicStringViewEmpty)
{
    constexpr basic_string_view<char> sv1;
    R_ASSERT_TRUE(sv1.empty());

    const auto str = "Hello";
    const basic_string_view<char> sv2(str);
    R_ASSERT_FALSE(sv2.empty());
}

TEST_F(StringViewTest, BasicStringViewSize)
{
    constexpr basic_string_view<char> sv1;
    R_ASSERT_EQ(0, sv1.size());

    const auto str = "Hello, World!";
    const basic_string_view<char> sv2(str);
    R_ASSERT_EQ(13, sv2.size());

    const basic_string_view<char> sv3(str, 5);
    R_ASSERT_EQ(5, sv3.size());
}

TEST_F(StringViewTest, BasicStringViewFrontAndBack)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    R_ASSERT_EQ('H', sv.front());
    R_ASSERT_EQ('!', sv.back());
}

TEST_F(StringViewTest, BasicStringViewStartsWithAndEndsWith)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    R_ASSERT_TRUE(sv.starts_with("Hello"));
    R_ASSERT_FALSE(sv.starts_with("World"));
    R_ASSERT_TRUE(sv.ends_with("World!"));
    R_ASSERT_FALSE(sv.ends_with("Hello"));

    R_ASSERT_TRUE(sv.starts_with('H'));
    R_ASSERT_FALSE(sv.starts_with('W'));
    R_ASSERT_TRUE(sv.ends_with('!'));
    R_ASSERT_FALSE(sv.ends_with('d'));
}

TEST_F(StringViewTest, BasicStringViewSwap)
{
    const auto str1 = "Hello";
    const auto str2 = "World!";

    basic_string_view<char> sv1(str1);
    basic_string_view<char> sv2(str2);

    sv1.swap(sv2);
    R_ASSERT_EQ(str2, sv1.data());
    R_ASSERT_EQ(6, sv1.size());
    R_ASSERT_EQ(str1, sv2.data());
    R_ASSERT_EQ(5, sv2.size());
}

TEST_F(StringViewTest, BasicStringViewRemovePrefixAndSuffix)
{
    const auto str = "Hello, World!";
    basic_string_view<char> sv(str);

    sv.remove_prefix(7);
    R_ASSERT_EQ(str + 7, sv.data());
    R_ASSERT_EQ(6, sv.size());
    R_ASSERT_EQ(0, sv.compare("World!"));

    sv.remove_suffix(1);
    R_ASSERT_EQ(5, sv.size());
    R_ASSERT_EQ(0, sv.compare("World"));
}

TEST_F(StringViewTest, BasicStringViewFindFirstOfWithEmptyString)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    R_ASSERT_EQ(basic_string_view<char>::npos, sv.find_first_of(basic_string_view<char>()));
    R_ASSERT_EQ(0, sv.find_first_of('H'));
    R_ASSERT_EQ(basic_string_view<char>::npos, sv.find_first_of('x'));
}

TEST_F(StringViewTest, BasicStringViewFindLastOfWithEmptyString)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    R_ASSERT_EQ(basic_string_view<char>::npos, sv.find_last_of(basic_string_view<char>()));
    R_ASSERT_EQ(12, sv.find_last_of('!'));
    R_ASSERT_EQ(basic_string_view<char>::npos, sv.find_last_of('x'));
}

TEST_F(StringViewTest, BasicStringViewFindFirstNotOfWithEmptyString)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    R_ASSERT_EQ(0, sv.find_first_not_of(basic_string_view<char>()));
    R_ASSERT_EQ(1, sv.find_first_not_of('H'));
    R_ASSERT_EQ(0, sv.find_first_not_of('x'));
}

TEST_F(StringViewTest, BasicStringViewFindLastNotOfWithEmptyString)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    R_ASSERT_EQ(12, sv.find_last_not_of(basic_string_view<char>()));
    R_ASSERT_EQ(11, sv.find_last_not_of('!'));
    R_ASSERT_EQ(12, sv.find_last_not_of('x'));
}

TEST_F(StringViewTest, BasicStringViewCopy)
{
    const auto str = "Hello, World!";
    const basic_string_view<char> sv(str);

    char buffer[20];
    const size_t copied = sv.copy(buffer, 5);
    buffer[copied]      = '\0';

    R_ASSERT_EQ(5, copied);
    R_ASSERT_EQ(0, strcmp(buffer, "Hello"));
}

TEST_F(StringViewTest, BasicStringViewAssignment)
{
    const auto str1 = "Hello, World!";
    const auto str2 = "Goodbye, World!";
    const basic_string_view<char> sv1(str1);
    basic_string_view<char> sv2;

    sv2 = sv1;
    R_ASSERT_EQ(str1, sv2.data());
    R_ASSERT_EQ(13, sv2.size());

    sv2 = basic_string_view<char>(str2);
    R_ASSERT_EQ(str2, sv2.data());
    R_ASSERT_EQ(15, sv2.size());
}

TEST_F(StringViewTest, BasicStringViewComparisonOperators)
{
    const auto str1 = "Hello, World!";
    const auto str2 = "Hello, World!";
    const auto str3 = "Goodbye, World!";
    basic_string_view<char> sv1(str1);
    basic_string_view<char> sv2(str2);
    basic_string_view<char> sv3(str3);

    R_ASSERT_TRUE(sv1 == sv2);
    R_ASSERT_FALSE(sv1 == sv3);
    R_ASSERT_FALSE(sv2 == sv3);

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

TEST_F(StringViewTest, BasicStringViewSwapFunction)
{
    const auto str1 = "Hello";
    const auto str2 = "World";
    basic_string_view<char> sv1(str1);
    basic_string_view<char> sv2(str2);

    swap(sv1, sv2);
    R_ASSERT_EQ(str2, sv1.data());
    R_ASSERT_EQ(5, sv1.size());
    R_ASSERT_EQ(str1, sv2.data());
    R_ASSERT_EQ(5, sv2.size());
}

TEST_F(StringViewTest, BasicStringViewLiterals)
{
    using namespace std::literals::string_view_literals;

    auto sv = "Hello, World!"sv;
    R_ASSERT_EQ(13, sv.size());
    R_ASSERT_EQ("Hello, World!", sv.data());
}
