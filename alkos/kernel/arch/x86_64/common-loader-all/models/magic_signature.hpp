#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_MODELS_MAGIC_SIGNATURE_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_MODELS_MAGIC_SIGNATURE_HPP_

#include <defines.hpp>
#include <extensions/types.hpp>

namespace Detail
{
// A constexpr string literal type that can be used as a non-type template parameter in C++20.
template <size_t kSize>
struct StringLiteral {
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    constexpr StringLiteral(const char (&str)[kSize])
    {
        for (size_t i = 0; i < kSize; ++i) {
            value[i] = str[i];
        }
    }

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    constexpr operator const char*() const { return value; }

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//

    static constexpr size_t size = kSize - 1;
    alignas(64) char value[kSize]{};
};

// Deduction guide for StringLiteral to allow deduction from array literals.
template <size_t N>
StringLiteral(const char (&)[N]) -> StringLiteral<N>;

}  // namespace Detail

template <Detail::StringLiteral MagicMessage>
class MagicSignature
{
    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    MagicSignature()                                 = default;
    MagicSignature(const MagicSignature&)            = delete;
    MagicSignature(MagicSignature&&)                 = delete;
    MagicSignature& operator=(const MagicSignature&) = delete;
    MagicSignature& operator=(MagicSignature&&)      = delete;

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    /// Checks if the provided magic string matches the expected magic string.
    /// Invalid if some data corruption happened.
    constexpr static bool IsValid()
    {
        for (size_t i = 0; i < sizeof(MagicMessage) - 1; ++i) {
            if (magic[i] != MagicMessage[i]) {
                return false;
            }
        }
        return true;
    }

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//

    constexpr static const char* magic =
        MagicMessage;  ///< The magic string that identifies the signature
};

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_MODELS_MAGIC_SIGNATURE_HPP_
