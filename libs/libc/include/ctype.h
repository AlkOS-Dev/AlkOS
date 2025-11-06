#ifndef LIBS_LIBC_INCLUDE_CTYPE_H_
#define LIBS_LIBC_INCLUDE_CTYPE_H_

#include <defines.h>
#include <types.hpp>

namespace internal
{

CONSTEXPR u8 kLower      = 0x01;
CONSTEXPR u8 kDigit      = 0x10;
CONSTEXPR u8 kUpper      = 0x02;
CONSTEXPR u8 kSpace      = 0x20;
CONSTEXPR u8 kXDigit     = 0x04;
CONSTEXPR u8 kExtraSpace = 0x40;
CONSTEXPR u8 kCntrl      = 0x08;
CONSTEXPR u8 kPunct      = 0x80;

CONSTEXPR u8 kAlpha = kLower | kUpper;
CONSTEXPR u8 kAlnum = kAlpha | kDigit;
CONSTEXPR u8 kGraph = kAlnum | kPunct;
CONSTEXPR u8 kPrint = kGraph | kExtraSpace;

CONSTEXPR u8 kLookup[256] = {
    kCntrl,               /* 0x00     0 NUL */
    kCntrl,               /* 0x01     1 SOH */
    kCntrl,               /* 0x02     2 STX */
    kCntrl,               /* 0x03     3 ETX */
    kCntrl,               /* 0x04     4 EOT */
    kCntrl,               /* 0x05     5 ENQ */
    kCntrl,               /* 0x06     6 ACK */
    kCntrl,               /* 0x07     7 BEL */
    kCntrl,               /* 0x08     8 BS  */
    kCntrl | kSpace,      /* 0x09     9 TAB */
    kCntrl | kSpace,      /* 0x0A    10 LF  */
    kCntrl | kSpace,      /* 0x0B    11 VT  */
    kCntrl | kSpace,      /* 0x0C    12 FF  */
    kCntrl | kSpace,      /* 0x0D    13 CR  */
    kCntrl,               /* 0x0E    14 SO  */
    kCntrl,               /* 0x0F    15 SI  */
    kCntrl,               /* 0x10    16 DLE */
    kCntrl,               /* 0x11    17 DC1 */
    kCntrl,               /* 0x12    18 DC2 */
    kCntrl,               /* 0x13    19 DC3 */
    kCntrl,               /* 0x14    20 DC4 */
    kCntrl,               /* 0x15    21 NAK */
    kCntrl,               /* 0x16    22 SYN */
    kCntrl,               /* 0x17    23 ETB */
    kCntrl,               /* 0x18    24 CAN */
    kCntrl,               /* 0x19    25 EM  */
    kCntrl,               /* 0x1A    26 SUB */
    kCntrl,               /* 0x1B    27 ESC */
    kCntrl,               /* 0x1C    28 FS  */
    kCntrl,               /* 0x1D    29 GS  */
    kCntrl,               /* 0x1E    30 RS  */
    kCntrl,               /* 0x1F    31 US  */
    kExtraSpace | kSpace, /* 0x20    32 ' ' */
    kPunct,               /* 0x21    33 '!' */
    kPunct,               /* 0x22    34 '"' */
    kPunct,               /* 0x23    35 '#' */
    kPunct,               /* 0x24    36 '$' */
    kPunct,               /* 0x25    37 '%' */
    kPunct,               /* 0x26    38 '&' */
    kPunct,               /* 0x27    39 ''' */
    kPunct,               /* 0x28    40 '(' */
    kPunct,               /* 0x29    41 ')' */
    kPunct,               /* 0x2A    42 '*' */
    kPunct,               /* 0x2B    43 '+' */
    kPunct,               /* 0x2C    44 ',' */
    kPunct,               /* 0x2D    45 '-' */
    kPunct,               /* 0x2E    46 '.' */
    kPunct,               /* 0x2F    47 '/' */
    kXDigit | kDigit,     /* 0x30    48 '0' */
    kXDigit | kDigit,     /* 0x31    49 '1' */
    kXDigit | kDigit,     /* 0x32    50 '2' */
    kXDigit | kDigit,     /* 0x33    51 '3' */
    kXDigit | kDigit,     /* 0x34    52 '4' */
    kXDigit | kDigit,     /* 0x35    53 '5' */
    kXDigit | kDigit,     /* 0x36    54 '6' */
    kXDigit | kDigit,     /* 0x37    55 '7' */
    kXDigit | kDigit,     /* 0x38    56 '8' */
    kXDigit | kDigit,     /* 0x39    57 '9' */
    kPunct,               /* 0x3A    58 ':' */
    kPunct,               /* 0x3B    59 ';' */
    kPunct,               /* 0x3C    60 '<' */
    kPunct,               /* 0x3D    61 '=' */
    kPunct,               /* 0x3E    62 '>' */
    kPunct,               /* 0x3F    63 '?' */
    kPunct,               /* 0x40    64 '@' */
    kXDigit | kUpper,     /* 0x41    65 'A' */
    kXDigit | kUpper,     /* 0x42    66 'B' */
    kXDigit | kUpper,     /* 0x43    67 'C' */
    kXDigit | kUpper,     /* 0x44    68 'D' */
    kXDigit | kUpper,     /* 0x45    69 'E' */
    kXDigit | kUpper,     /* 0x46    70 'F' */
    kUpper,               /* 0x47    71 'G' */
    kUpper,               /* 0x48    72 'H' */
    kUpper,               /* 0x49    73 'I' */
    kUpper,               /* 0x4A    74 'J' */
    kUpper,               /* 0x4B    75 'K' */
    kUpper,               /* 0x4C    76 'L' */
    kUpper,               /* 0x4D    77 'M' */
    kUpper,               /* 0x4E    78 'N' */
    kUpper,               /* 0x4F    79 'O' */
    kUpper,               /* 0x50    80 'P' */
    kUpper,               /* 0x51    81 'Q' */
    kUpper,               /* 0x52    82 'R' */
    kUpper,               /* 0x53    83 'S' */
    kUpper,               /* 0x54    84 'T' */
    kUpper,               /* 0x55    85 'U' */
    kUpper,               /* 0x56    86 'V' */
    kUpper,               /* 0x57    87 'W' */
    kUpper,               /* 0x58    88 'X' */
    kUpper,               /* 0x59    89 'Y' */
    kUpper,               /* 0x5A    90 'Z' */
    kPunct,               /* 0x5B    91 '[' */
    kPunct,               /* 0x5C    92 '\' */
    kPunct,               /* 0x5D    93 ']' */
    kPunct,               /* 0x5E    94 '^' */
    kPunct,               /* 0x5F    95 '_' */
    kPunct,               /* 0x60    96 '`' */
    kXDigit | kLower,     /* 0x61    97 'a' */
    kXDigit | kLower,     /* 0x62    98 'b' */
    kXDigit | kLower,     /* 0x63    99 'c' */
    kXDigit | kLower,     /* 0x64   100 'd' */
    kXDigit | kLower,     /* 0x65   101 'e' */
    kXDigit | kLower,     /* 0x66   102 'f' */
    kLower,               /* 0x67   103 'g' */
    kLower,               /* 0x68   104 'h' */
    kLower,               /* 0x69   105 'i' */
    kLower,               /* 0x6A   106 'j' */
    kLower,               /* 0x6B   107 'k' */
    kLower,               /* 0x6C   108 'l' */
    kLower,               /* 0x6D   109 'm' */
    kLower,               /* 0x6E   110 'n' */
    kLower,               /* 0x6F   111 'o' */
    kLower,               /* 0x70   112 'p' */
    kLower,               /* 0x71   113 'q' */
    kLower,               /* 0x72   114 'r' */
    kLower,               /* 0x73   115 's' */
    kLower,               /* 0x74   116 't' */
    kLower,               /* 0x75   117 'u' */
    kLower,               /* 0x76   118 'v' */
    kLower,               /* 0x77   119 'w' */
    kLower,               /* 0x78   120 'x' */
    kLower,               /* 0x79   121 'y' */
    kLower,               /* 0x7A   122 'z' */
    kPunct,               /* 0x7B   123 '{' */
    kPunct,               /* 0x7C   124 '|' */
    kPunct,               /* 0x7D   125 '}' */
    kPunct,               /* 0x7E   126 '~' */
    kCntrl,               /* 0x7F   127 DEL */
};

}  // namespace internal

BEGIN_DECL_C

CONSTEXPR int isalnum(int c)
{
    return internal::kLookup[static_cast<unsigned char>(c)] & internal::kAlnum;
}
CONSTEXPR int isalpha(int c)
{
    return internal::kLookup[static_cast<unsigned char>(c)] & internal::kAlpha;
}
CONSTEXPR int iscntrl(int c)
{
    return internal::kLookup[static_cast<unsigned char>(c)] & internal::kCntrl;
}
CONSTEXPR int isdigit(int c)
{
    return internal::kLookup[static_cast<unsigned char>(c)] & internal::kDigit;
}
CONSTEXPR int isgraph(int c)
{
    return internal::kLookup[static_cast<unsigned char>(c)] & internal::kGraph;
}
CONSTEXPR int islower(int c)
{
    return internal::kLookup[static_cast<unsigned char>(c)] & internal::kLower;
}
CONSTEXPR int isprint(int c)
{
    return internal::kLookup[static_cast<unsigned char>(c)] & internal::kPrint;
}
CONSTEXPR int ispunct(int c)
{
    return internal::kLookup[static_cast<unsigned char>(c)] & internal::kPunct;
}
CONSTEXPR int isspace(int c)
{
    return internal::kLookup[static_cast<unsigned char>(c)] & internal::kSpace;
}
CONSTEXPR int isupper(int c)
{
    return internal::kLookup[static_cast<unsigned char>(c)] & internal::kUpper;
}
CONSTEXPR int isxdigit(int c)
{
    return internal::kLookup[static_cast<unsigned char>(c)] & internal::kXDigit;
}
CONSTEXPR int tolower(int c) { return isupper(c) ? c + 0x20 : c; }
CONSTEXPR int toupper(int c) { return islower(c) ? c - 0x20 : c; }

END_DECL_C

#endif  // LIBS_LIBC_INCLUDE_CTYPE_H_
