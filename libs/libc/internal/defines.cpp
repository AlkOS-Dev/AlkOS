#include <compare.hpp>
#include <defines.hpp>

template <size_t ID, size_t OtherID>
constexpr bool operator==(empty_t<ID>, empty_t<OtherID>) noexcept
{
    return true;
}

template <size_t ID, size_t OtherID>
constexpr std::strong_ordering operator<=>(empty_t<ID>, empty_t<OtherID>) noexcept
{
    return std::strong_ordering::equal;
}
