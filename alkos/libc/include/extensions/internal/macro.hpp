#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_MACRO_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_MACRO_HPP_

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

// https://www.scs.stanford.edu/~dm/blog/va-opt.html
#define PARENS ()

#define EXPAND(...)  EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...) \
    macro(a1) __VA_OPT__(, FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define FOR_EACH_PAIR(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_PAIR_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_PAIR_HELPER(macro, a1, a2, ...) \
    macro(a1, a2) __VA_OPT__(, FOR_EACH_PAIR_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_AGAIN() FOR_EACH_PAIR_HELPER

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_MACRO_HPP_
