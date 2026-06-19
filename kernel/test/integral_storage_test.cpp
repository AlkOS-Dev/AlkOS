// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <bit.hpp>
#include <bits_ext.hpp>
#include <template/integral_storage.hpp>
#include <test_module/test.hpp>

using namespace template_lib;

class MinimalUnsignedStorageTest : public TestGroupBase
{
};

TEST_F(MinimalUnsignedStorageTest, MinimalUnsignedStorage_AllBoundariesAndEdgeCases)
{
    constexpr size_t val_lsb         = kLsb<size_t>;
    constexpr size_t val_bit1        = kSingleBit<size_t, 1>;
    constexpr size_t val_bit4        = kBitMask<size_t, 0, 4>;
    constexpr size_t val_bit7        = kBitMask<size_t, 0, 7>;
    constexpr size_t val_8bits       = static_cast<size_t>(kBitMask8);
    constexpr size_t val_8bits_plus1 = static_cast<size_t>(kBitMask8) + 1;
    constexpr size_t val_bit9        = kSingleBit<size_t, 9>;
    constexpr size_t val_bit15       = kBitMask<size_t, 0, 15>;
    constexpr size_t val_16bits      = static_cast<size_t>(kBitMask16);
    constexpr size_t val_bit25       = kSingleBit<size_t, 25>;
    constexpr size_t val_bit31       = kBitMask<size_t, 0, 31>;
    constexpr size_t val_32bits      = static_cast<size_t>(kBitMask32);
    constexpr size_t val_bit63       = kBitMask<size_t, 0, 63>;
    constexpr size_t val_64bits      = static_cast<size_t>(kBitMask64);

    using t_lsb         = MinimalUnsignedStorage_t<val_lsb>;
    using t_bit1        = MinimalUnsignedStorage_t<val_bit1>;
    using t_bit4        = MinimalUnsignedStorage_t<val_bit4>;
    using t_bit7        = MinimalUnsignedStorage_t<val_bit7>;
    using t_8bits       = MinimalUnsignedStorage_t<val_8bits>;
    using t_8bits_plus1 = MinimalUnsignedStorage_t<val_8bits_plus1>;
    using t_bit9        = MinimalUnsignedStorage_t<val_bit9>;
    using t_bit15       = MinimalUnsignedStorage_t<val_bit15>;
    using t_16bits      = MinimalUnsignedStorage_t<val_16bits>;
    using t_bit25       = MinimalUnsignedStorage_t<val_bit25>;
    using t_bit31       = MinimalUnsignedStorage_t<val_bit31>;
    using t_32bits      = MinimalUnsignedStorage_t<val_32bits>;
    using t_bit63       = MinimalUnsignedStorage_t<val_bit63>;
    using t_64bits      = MinimalUnsignedStorage_t<val_64bits>;

    EXPECT_TRUE((std::is_same_v<t_lsb, u8>));
    EXPECT_TRUE((std::is_same_v<t_bit1, u8>));
    EXPECT_TRUE((std::is_same_v<t_bit4, u8>));
    EXPECT_TRUE((std::is_same_v<t_bit7, u8>));
    EXPECT_TRUE((std::is_same_v<t_8bits, u8>));
    EXPECT_TRUE((std::is_same_v<t_8bits_plus1, u16>));
    EXPECT_TRUE((std::is_same_v<t_bit9, u16>));
    EXPECT_TRUE((std::is_same_v<t_bit15, u16>));
    EXPECT_TRUE((std::is_same_v<t_16bits, u16>));
    EXPECT_TRUE((std::is_same_v<t_bit25, u32>));
    EXPECT_TRUE((std::is_same_v<t_bit31, u32>));
    EXPECT_TRUE((std::is_same_v<t_32bits, u32>));
    EXPECT_TRUE((std::is_same_v<t_bit63, u64>));
    EXPECT_TRUE((std::is_same_v<t_64bits, u64>));
}
