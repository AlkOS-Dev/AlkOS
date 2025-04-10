#include <ctype.h>
#include <test_module/test.hpp>

class CTypeTest : public TestGroupBase
{
};

// ------------------------------
// isalnum tests
// ------------------------------

TEST_F(CTypeTest, isalnum)
{
    // Basic alphanumeric checks
    R_ASSERT_GE(isalnum('a'), 0);
    R_ASSERT_GE(isalnum('z'), 0);
    R_ASSERT_GE(isalnum('A'), 0);
    R_ASSERT_GE(isalnum('Z'), 0);
    R_ASSERT_GE(isalnum('0'), 0);
    R_ASSERT_GE(isalnum('9'), 0);

    // Non-alphanumeric checks
    R_ASSERT_ZERO(isalnum(' '));
    R_ASSERT_ZERO(isalnum('\n'));
    R_ASSERT_ZERO(isalnum('@'));
}

// ------------------------------
// isalpha tests
// ------------------------------

TEST_F(CTypeTest, isalpha)
{
    // Lowercase letters
    R_ASSERT_GE(isalpha('a'), 0);
    R_ASSERT_GE(isalpha('m'), 0);
    R_ASSERT_GE(isalpha('z'), 0);

    // Uppercase letters
    R_ASSERT_GE(isalpha('A'), 0);
    R_ASSERT_GE(isalpha('M'), 0);
    R_ASSERT_GE(isalpha('Z'), 0);

    // Non-letters
    R_ASSERT_ZERO(isalpha('1'));
    R_ASSERT_ZERO(isalpha(' '));
    R_ASSERT_ZERO(isalpha('\n'));
}

// ------------------------------
// iscntrl tests
// ------------------------------

TEST_F(CTypeTest, iscntrl)
{
    // Control characters
    R_ASSERT_GE(iscntrl('\0'), 0);
    R_ASSERT_GE(iscntrl('\n'), 0);
    R_ASSERT_GE(iscntrl('\r'), 0);
    R_ASSERT_GE(iscntrl('\x1F'), 0);
    R_ASSERT_GE(iscntrl('\x7F'), 0);

    // Non-control characters
    R_ASSERT_ZERO(iscntrl('A'));
    R_ASSERT_ZERO(iscntrl('1'));
    R_ASSERT_ZERO(iscntrl(' '));
}

// ------------------------------
// isdigit tests
// ------------------------------

TEST_F(CTypeTest, isdigit)
{
    // Numeric digits
    R_ASSERT_GE(isdigit('0'), 0);
    R_ASSERT_GE(isdigit('5'), 0);
    R_ASSERT_GE(isdigit('9'), 0);

    // Non-digits
    R_ASSERT_ZERO(isdigit('a'));
    R_ASSERT_ZERO(isdigit('A'));
    R_ASSERT_ZERO(isdigit(' '));
}

// ------------------------------
// isgraph tests
// ------------------------------

TEST_F(CTypeTest, isgraph)
{
    // Visible characters
    R_ASSERT_GE(isgraph('A'), 0);
    R_ASSERT_GE(isgraph('1'), 0);
    R_ASSERT_GE(isgraph('!'), 0);
    R_ASSERT_GE(isgraph('~'), 0);

    // Non-visible characters
    R_ASSERT_ZERO(isgraph(' '));
    R_ASSERT_ZERO(isgraph('\n'));
    R_ASSERT_ZERO(isgraph('\0'));
}

// ------------------------------
// islower tests
// ------------------------------

TEST_F(CTypeTest, islower)
{
    // Lowercase letters
    R_ASSERT_GE(islower('a'), 0);
    R_ASSERT_GE(islower('n'), 0);
    R_ASSERT_GE(islower('z'), 0);

    // Non-lowercase
    R_ASSERT_ZERO(islower('A'));
    R_ASSERT_ZERO(islower('1'));
    R_ASSERT_ZERO(islower(' '));
}

// ------------------------------
// isprint tests
// ------------------------------

TEST_F(CTypeTest, isprint)
{
    // Printable characters
    R_ASSERT_GE(isprint('A'), 0);
    R_ASSERT_GE(isprint('1'), 0);
    R_ASSERT_GE(isprint(' '), 0);
    R_ASSERT_GE(isprint('~'), 0);

    // Non-printable characters
    R_ASSERT_ZERO(isprint('\n'));
    R_ASSERT_ZERO(isprint('\0'));
    R_ASSERT_ZERO(isprint('\x7F'));
}

// ------------------------------
// ispunct tests
// ------------------------------

TEST_F(CTypeTest, ispunct)
{
    // Punctuation characters
    R_ASSERT_GE(ispunct('.'), 0);
    R_ASSERT_GE(ispunct(','), 0);
    R_ASSERT_GE(ispunct('!'), 0);
    R_ASSERT_GE(ispunct('@'), 0);

    // Non-punctuation
    R_ASSERT_ZERO(ispunct('A'));
    R_ASSERT_ZERO(ispunct('1'));
    R_ASSERT_ZERO(ispunct(' '));
}

// ------------------------------
// isspace tests
// ------------------------------

TEST_F(CTypeTest, isspace)
{
    // Whitespace characters
    R_ASSERT_GE(isspace(' '), 0);
    R_ASSERT_GE(isspace('\t'), 0);
    R_ASSERT_GE(isspace('\n'), 0);
    R_ASSERT_GE(isspace('\v'), 0);
    R_ASSERT_GE(isspace('\f'), 0);
    R_ASSERT_GE(isspace('\r'), 0);

    // Non-whitespace
    R_ASSERT_ZERO(isspace('A'));
    R_ASSERT_ZERO(isspace('1'));
    R_ASSERT_ZERO(isspace('_'));
}

// ------------------------------
// isupper tests
// ------------------------------

TEST_F(CTypeTest, isupper)
{
    // Uppercase letters
    R_ASSERT_GE(isupper('A'), 0);
    R_ASSERT_GE(isupper('N'), 0);
    R_ASSERT_GE(isupper('Z'), 0);

    // Non-uppercase
    R_ASSERT_ZERO(isupper('a'));
    R_ASSERT_ZERO(isupper('1'));
    R_ASSERT_ZERO(isupper(' '));
}

// ------------------------------
// isxdigit tests
// ------------------------------

TEST_F(CTypeTest, isxdigit)
{
    // Hexadecimal digits
    R_ASSERT_GE(isxdigit('0'), 0);
    R_ASSERT_GE(isxdigit('9'), 0);
    R_ASSERT_GE(isxdigit('a'), 0);
    R_ASSERT_GE(isxdigit('f'), 0);
    R_ASSERT_GE(isxdigit('A'), 0);
    R_ASSERT_GE(isxdigit('F'), 0);

    // Non-hex digits
    R_ASSERT_ZERO(isxdigit('g'));
    R_ASSERT_ZERO(isxdigit('G'));
    R_ASSERT_ZERO(isxdigit(' '));
}

// ------------------------------
// tolower tests
// ------------------------------

TEST_F(CTypeTest, tolower)
{
    // Uppercase to lowercase
    R_ASSERT_EQ(tolower('A'), 'a');
    R_ASSERT_EQ(tolower('Z'), 'z');

    // Already lowercase
    R_ASSERT_EQ(tolower('a'), 'a');
    R_ASSERT_EQ(tolower('z'), 'z');

    // Non-letters unchanged
    R_ASSERT_EQ(tolower('0'), '0');
    R_ASSERT_EQ(tolower(' '), ' ');
    R_ASSERT_EQ(tolower('\n'), '\n');
}

// ------------------------------
// toupper tests
// ------------------------------

TEST_F(CTypeTest, toupper)
{
    // Lowercase to uppercase
    R_ASSERT_EQ(toupper('a'), 'A');
    R_ASSERT_EQ(toupper('z'), 'Z');

    // Already uppercase
    R_ASSERT_EQ(toupper('A'), 'A');
    R_ASSERT_EQ(toupper('Z'), 'Z');

    // Non-letters unchanged
    R_ASSERT_EQ(toupper('0'), '0');
    R_ASSERT_EQ(toupper(' '), ' ');
    R_ASSERT_EQ(toupper('\n'), '\n');
}
