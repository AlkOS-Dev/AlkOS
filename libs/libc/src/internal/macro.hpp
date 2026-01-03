#ifndef LIBS_LIBC_SRC_INTERNAL_MACRO_HPP_
#define LIBS_LIBC_SRC_INTERNAL_MACRO_HPP_

// https://www.scs.stanford.edu/~dm/blog/va-opt.html
#define PARENS      ()
#define SEMICOLON() ;
#define COMMA()     ,

#define EXPAND(...)  EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...) \
    macro(a1) __VA_OPT__(, FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define FOR_EACH_PAIR_SEP(sep, macro, ...) \
    __VA_OPT__(EXPAND(FOR_EACH_PAIR_SEP_HELPER(sep, macro, __VA_ARGS__)))
#define FOR_EACH_PAIR_SEP_HELPER(sep, macro, a1, a2, ...) \
    macro(a1, a2) __VA_OPT__(sep PARENS FOR_EACH_PAIR_SEP_AGAIN PARENS(sep, macro, __VA_ARGS__))
#define FOR_EACH_PAIR_SEP_AGAIN() FOR_EACH_PAIR_SEP_HELPER

#define FOR_EACH_PAIR(macro, ...) FOR_EACH_PAIR_SEP(COMMA, macro, __VA_ARGS__)

#define FOR_EACH_PAIR_CTX_SEP(sep, macro, ctx, ...) \
    __VA_OPT__(EXPAND(FOR_EACH_PAIR_CTX_SEP_HELPER(sep, macro, ctx, __VA_ARGS__)))
#define FOR_EACH_PAIR_CTX_SEP_HELPER(sep, macro, ctx, a1, a2, ...) \
    macro(ctx, a1, a2)                                             \
        __VA_OPT__(sep PARENS FOR_EACH_PAIR_CTX_SEP_AGAIN PARENS(sep, macro, ctx, __VA_ARGS__))
#define FOR_EACH_PAIR_CTX_SEP_AGAIN() FOR_EACH_PAIR_CTX_SEP_HELPER

#define FOR_EACH_PAIR_CTX(macro, ctx, ...) FOR_EACH_PAIR_CTX_SEP(COMMA, macro, ctx, __VA_ARGS__)

#define MERGE(x, y)          x y
#define GET_FIRST_ARG(x, y)  x
#define GET_SECOND_ARG(x, y) y

#endif  // LIBS_LIBC_SRC_INTERNAL_MACRO_HPP_
