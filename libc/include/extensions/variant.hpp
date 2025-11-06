#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_VARIANT_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_VARIANT_HPP_

namespace std
{
// ------------------------------
// std::monostate
// ------------------------------

struct monostate {
};
constexpr bool operator==(monostate, monostate) noexcept { return true; }
constexpr strong_ordering operator<=>(monostate, monostate) noexcept
{
    return strong_ordering::equal;
}
}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_VARIANT_HPP_
