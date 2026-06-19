// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INCLUDE_CTYPE_H_
#define LIBS_LIBC_SRC_INCLUDE_CTYPE_H_

#include <defines.h>
#include <types.h>

#define _K_LOWER  0x01
#define _K_UPPER  0x02
#define _K_XDIGIT 0x04
#define _K_CNTRL  0x08
#define _K_DIGIT  0x10
#define _K_ALPHA  (_K_LOWER | _K_UPPER)
#define _K_ALNUM  (_K_ALPHA | _K_DIGIT)
#define _K_PUNCT  0x80
#define _K_SPACE  0x20
#define _K_EXTRA  0x40
#define _K_GRAPH  (_K_ALNUM | _K_PUNCT)
#define _K_PRINT  (_K_GRAPH | _K_EXTRA)

static const u8 __ctype_lookup[256] = {
    _K_CNTRL,             /* 0x00     0 NUL */
    _K_CNTRL,             /* 0x01     1 SOH */
    _K_CNTRL,             /* 0x02     2 STX */
    _K_CNTRL,             /* 0x03     3 ETX */
    _K_CNTRL,             /* 0x04     4 EOT */
    _K_CNTRL,             /* 0x05     5 ENQ */
    _K_CNTRL,             /* 0x06     6 ACK */
    _K_CNTRL,             /* 0x07     7 BEL */
    _K_CNTRL,             /* 0x08     8 BS  */
    _K_CNTRL | _K_SPACE,  /* 0x09     9 TAB */
    _K_CNTRL | _K_SPACE,  /* 0x0A    10 LF  */
    _K_CNTRL | _K_SPACE,  /* 0x0B    11 VT  */
    _K_CNTRL | _K_SPACE,  /* 0x0C    12 FF  */
    _K_CNTRL | _K_SPACE,  /* 0x0D    13 CR  */
    _K_CNTRL,             /* 0x0E    14 SO  */
    _K_CNTRL,             /* 0x0F    15 SI  */
    _K_CNTRL,             /* 0x10    16 DLE */
    _K_CNTRL,             /* 0x11    17 DC1 */
    _K_CNTRL,             /* 0x12    18 DC2 */
    _K_CNTRL,             /* 0x13    19 DC3 */
    _K_CNTRL,             /* 0x14    20 DC4 */
    _K_CNTRL,             /* 0x15    21 NAK */
    _K_CNTRL,             /* 0x16    22 SYN */
    _K_CNTRL,             /* 0x17    23 ETB */
    _K_CNTRL,             /* 0x18    24 CAN */
    _K_CNTRL,             /* 0x19    25 EM  */
    _K_CNTRL,             /* 0x1A    26 SUB */
    _K_CNTRL,             /* 0x1B    27 ESC */
    _K_CNTRL,             /* 0x1C    28 FS  */
    _K_CNTRL,             /* 0x1D    29 GS  */
    _K_CNTRL,             /* 0x1E    30 RS  */
    _K_CNTRL,             /* 0x1F    31 US  */
    _K_SPACE | _K_EXTRA,  /* 0x20    32 ' ' */
    _K_PUNCT,             /* 0x21    33 '!' */
    _K_PUNCT,             /* 0x22    34 '"' */
    _K_PUNCT,             /* 0x23    35 '#' */
    _K_PUNCT,             /* 0x24    36 '$' */
    _K_PUNCT,             /* 0x25    37 '%' */
    _K_PUNCT,             /* 0x26    38 '&' */
    _K_PUNCT,             /* 0x27    39 ''' */
    _K_PUNCT,             /* 0x28    40 '(' */
    _K_PUNCT,             /* 0x29    41 ')' */
    _K_PUNCT,             /* 0x2A    42 '*' */
    _K_PUNCT,             /* 0x2B    43 '+' */
    _K_PUNCT,             /* 0x2C    44 ',' */
    _K_PUNCT,             /* 0x2D    45 '-' */
    _K_PUNCT,             /* 0x2E    46 '.' */
    _K_PUNCT,             /* 0x2F    47 '/' */
    _K_DIGIT | _K_XDIGIT, /* 0x30    48 '0' */
    _K_DIGIT | _K_XDIGIT, /* 0x31    49 '1' */
    _K_DIGIT | _K_XDIGIT, /* 0x32    50 '2' */
    _K_DIGIT | _K_XDIGIT, /* 0x33    51 '3' */
    _K_DIGIT | _K_XDIGIT, /* 0x34    52 '4' */
    _K_DIGIT | _K_XDIGIT, /* 0x35    53 '5' */
    _K_DIGIT | _K_XDIGIT, /* 0x36    54 '6' */
    _K_DIGIT | _K_XDIGIT, /* 0x37    55 '7' */
    _K_DIGIT | _K_XDIGIT, /* 0x38    56 '8' */
    _K_DIGIT | _K_XDIGIT, /* 0x39    57 '9' */
    _K_PUNCT,             /* 0x3A    58 ':' */
    _K_PUNCT,             /* 0x3B    59 ';' */
    _K_PUNCT,             /* 0x3C    60 '<' */
    _K_PUNCT,             /* 0x3D    61 '=' */
    _K_PUNCT,             /* 0x3E    62 '>' */
    _K_PUNCT,             /* 0x3F    63 '?' */
    _K_PUNCT,             /* 0x40    64 '@' */
    _K_UPPER | _K_XDIGIT, /* 0x41    65 'A' */
    _K_UPPER | _K_XDIGIT, /* 0x42    66 'B' */
    _K_UPPER | _K_XDIGIT, /* 0x43    67 'C' */
    _K_UPPER | _K_XDIGIT, /* 0x44    68 'D' */
    _K_UPPER | _K_XDIGIT, /* 0x45    69 'E' */
    _K_UPPER | _K_XDIGIT, /* 0x46    70 'F' */
    _K_UPPER,             /* 0x47    71 'G' */
    _K_UPPER,             /* 0x48    72 'H' */
    _K_UPPER,             /* 0x49    73 'I' */
    _K_UPPER,             /* 0x4A    74 'J' */
    _K_UPPER,             /* 0x4B    75 'K' */
    _K_UPPER,             /* 0x4C    76 'L' */
    _K_UPPER,             /* 0x4D    77 'M' */
    _K_UPPER,             /* 0x4E    78 'N' */
    _K_UPPER,             /* 0x4F    79 'O' */
    _K_UPPER,             /* 0x50    80 'P' */
    _K_UPPER,             /* 0x51    81 'Q' */
    _K_UPPER,             /* 0x52    82 'R' */
    _K_UPPER,             /* 0x53    83 'S' */
    _K_UPPER,             /* 0x54    84 'T' */
    _K_UPPER,             /* 0x55    85 'U' */
    _K_UPPER,             /* 0x56    86 'V' */
    _K_UPPER,             /* 0x57    87 'W' */
    _K_UPPER,             /* 0x58    88 'X' */
    _K_UPPER,             /* 0x59    89 'Y' */
    _K_UPPER,             /* 0x5A    90 'Z' */
    _K_PUNCT,             /* 0x5B    91 '[' */
    _K_PUNCT,             /* 0x5C    92 '\' */
    _K_PUNCT,             /* 0x5D    93 ']' */
    _K_PUNCT,             /* 0x5E    94 '^' */
    _K_PUNCT,             /* 0x5F    95 '_' */
    _K_PUNCT,             /* 0x60    96 '`' */
    _K_LOWER | _K_XDIGIT, /* 0x61    97 'a' */
    _K_LOWER | _K_XDIGIT, /* 0x62    98 'b' */
    _K_LOWER | _K_XDIGIT, /* 0x63    99 'c' */
    _K_LOWER | _K_XDIGIT, /* 0x64   100 'd' */
    _K_LOWER | _K_XDIGIT, /* 0x65   101 'e' */
    _K_LOWER | _K_XDIGIT, /* 0x66   102 'f' */
    _K_LOWER,             /* 0x67   103 'g' */
    _K_LOWER,             /* 0x68   104 'h' */
    _K_LOWER,             /* 0x69   105 'i' */
    _K_LOWER,             /* 0x6A   106 'j' */
    _K_LOWER,             /* 0x6B    107 'k' */
    _K_LOWER,             /* 0x6C   108 'l' */
    _K_LOWER,             /* 0x6D   109 'm' */
    _K_LOWER,             /* 0x6E   110 'n' */
    _K_LOWER,             /* 0x6F   111 'o' */
    _K_LOWER,             /* 0x70   112 'p' */
    _K_LOWER,             /* 0x71   113 'q' */
    _K_LOWER,             /* 0x72   114 'r' */
    _K_LOWER,             /* 0x73   115 's' */
    _K_LOWER,             /* 0x74   116 't' */
    _K_LOWER,             /* 0x75   117 'u' */
    _K_LOWER,             /* 0x76   118 'v' */
    _K_LOWER,             /* 0x77   119 'w' */
    _K_LOWER,             /* 0x78   120 'x' */
    _K_LOWER,             /* 0x79   121 'y' */
    _K_LOWER,             /* 0x7A   122 'z' */
    _K_PUNCT,             /* 0x7B   123 '{' */
    _K_PUNCT,             /* 0x7C   124 '|' */
    _K_PUNCT,             /* 0x7D   125 '}' */
    _K_PUNCT,             /* 0x7E   126 '~' */
    _K_CNTRL,             /* 0x7F   127 DEL */
};

BEGIN_DECL_C

CONSTEXPR int isalnum(int c) { return __ctype_lookup[(u8)c] & _K_ALNUM; }
CONSTEXPR int isalpha(int c) { return __ctype_lookup[(u8)c] & _K_ALPHA; }
CONSTEXPR int iscntrl(int c) { return __ctype_lookup[(u8)c] & _K_CNTRL; }
CONSTEXPR int isdigit(int c) { return __ctype_lookup[(u8)c] & _K_DIGIT; }
CONSTEXPR int isgraph(int c) { return __ctype_lookup[(u8)c] & _K_GRAPH; }
CONSTEXPR int islower(int c) { return __ctype_lookup[(u8)c] & _K_LOWER; }
CONSTEXPR int isprint(int c) { return __ctype_lookup[(u8)c] & _K_PRINT; }
CONSTEXPR int ispunct(int c) { return __ctype_lookup[(u8)c] & _K_PUNCT; }
CONSTEXPR int isspace(int c) { return __ctype_lookup[(u8)c] & _K_SPACE; }
CONSTEXPR int isupper(int c) { return __ctype_lookup[(u8)c] & _K_UPPER; }
CONSTEXPR int isxdigit(int c) { return __ctype_lookup[(u8)c] & _K_XDIGIT; }
CONSTEXPR int tolower(int c) { return isupper(c) ? c + 0x20 : c; }
CONSTEXPR int toupper(int c) { return islower(c) ? c - 0x20 : c; }

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_CTYPE_H_
