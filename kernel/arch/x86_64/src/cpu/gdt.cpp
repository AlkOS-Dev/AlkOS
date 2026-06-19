// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "gdt.hpp"

#include <bits_ext.hpp>

void cpu::DefaultEntryInit(
    GdtEntry<> &entry, const AccessByte access, const GdtEntry<>::Flags flags
)
{
    entry.limit      = 0xFFFF;
    entry.base       = 0;
    entry.base_mid   = 0;
    entry.access     = access;
    entry.limit_high = 0xF;
    entry.base_high  = 0;

    // Flags
    entry.reserved         = 0;
    entry.long_mode_flag   = flags.long_mode_flag;
    entry.granularity_flag = flags.granularity_flag;
    entry.size_flag        = flags.size_flag;
}

void cpu::DefaultTssEntryInit(GdtSystemSegmentDescriptor &entry, const u64 tss_address)
{
    static constexpr u32 kLimit = sizeof(TSS) - 1;

    // encode limit
    entry.low.limit      = kLimit & kBitMask16;
    entry.low.limit_high = (kLimit >> 16) & kBitMask4;

    // encode base
    entry.low.base      = tss_address & kBitMask16;
    entry.low.base_mid  = (tss_address >> 16) & kBitMask8;
    entry.low.base_high = (tss_address >> 24) & kBitMask8;
    entry.base_upper32  = (tss_address >> 32) & kBitMask32;

    // setup access
    entry.low.access.present_bit = SystemSegmentAccessByte::PresentBit::kPresent;
    entry.low.access.type_bit    = SystemSegmentAccessByte::DescriptorTypeBit::kSystemSegment;
    entry.low.access.dpl_bits    = SystemSegmentAccessByte::DescriptorPrivilegeLevel::kKernel;
    entry.low.access.type        = static_cast<u8>(SystemSegmentAccessByte::LongModeTypes::kTSS);

    entry.low.reserved = 0;
    entry.reserved     = 0;
}

void cpu::DefaultGdtInit(GDT &gdt, const u64 tss_address)
{
    AccessByte access_kernel_code{};
    access_kernel_code.present_bit    = AccessByte::PresentBit::kPresent;
    access_kernel_code.executable_bit = AccessByte::ExecutableBit::kExecutable;
    access_kernel_code.type_bit       = AccessByte::DescriptorTypeBit::kUsualSegment;
    access_kernel_code.dpl_bit        = AccessByte::DescriptorPrivilegeLevel::kKernel;
    GdtEntry<>::Flags flags{};
    flags.long_mode_flag = GdtEntry<>::LongModeFlag::kLongModeEnabled;

    DefaultEntryInit(gdt.kernel_code, access_kernel_code, flags);

    AccessByte access_kernel_data{};
    access_kernel_data.present_bit = AccessByte::PresentBit::kPresent;
    access_kernel_data.type_bit    = AccessByte::DescriptorTypeBit::kUsualSegment;
    access_kernel_data.rw_bit      = AccessByte::RWBit::kAllowed;
    access_kernel_data.dpl_bit     = AccessByte::DescriptorPrivilegeLevel::kKernel;

    DefaultEntryInit(gdt.kernel_data, access_kernel_data, {});

    AccessByte access_user_code{};
    access_user_code.present_bit    = AccessByte::PresentBit::kPresent;
    access_user_code.executable_bit = AccessByte::ExecutableBit::kExecutable;
    access_user_code.type_bit       = AccessByte::DescriptorTypeBit::kUsualSegment;
    access_user_code.dpl_bit        = AccessByte::DescriptorPrivilegeLevel::kUser;

    DefaultEntryInit(gdt.user_code, access_user_code, flags);

    AccessByte access_user_data{};
    access_user_data.present_bit = AccessByte::PresentBit::kPresent;
    access_user_data.type_bit    = AccessByte::DescriptorTypeBit::kUsualSegment;
    access_user_data.rw_bit      = AccessByte::RWBit::kAllowed;
    access_user_data.dpl_bit     = AccessByte::DescriptorPrivilegeLevel::kUser;

    DefaultEntryInit(gdt.user_data, access_user_data, {});

    DefaultTssEntryInit(gdt.tss_descriptor, tss_address);
}
