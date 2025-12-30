#ifndef KERNEL_ARCH_X86_64_SRC_CPU_GDT_HPP_
#define KERNEL_ARCH_X86_64_SRC_CPU_GDT_HPP_

#include <types.h>
#include <defines.hpp>
#include <type_traits.hpp>

#include "tss.hpp"

namespace cpu
{
struct PACK AccessByte {
    enum class AccessedBit : u8 {
        kNotAccessed = 0,
        kAccessed    = 1,
    };

    enum class RWBit : u8 {
        kUnallowed = 0,
        kAllowed   = 1,
    };

    enum class DirectionBit : u8 {
        kUp   = 0,
        kDown = 1,
    };

    enum class ConformingBit : u8 {
        kExecutionOnlyByDPL      = 0,
        kExecutionByLowerThanDPL = 1,
    };

    enum class ExecutableBit : u8 {
        kNonExecutable = 0,
        kExecutable    = 1,
    };

    enum class DescriptorTypeBit : u8 {
        kSystemSegment = 0,
        kUsualSegment  = 1,
    };

    enum class DescriptorPrivilegeLevel : u8 {
        kKernel = 0,
        kRing1  = 1,
        kRing2  = 2,
        kUser   = 3,
    };

    enum class PresentBit : u8 {
        kPresent    = 1,
        kNotPresent = 0,
    };

    AccessedBit accessed_bit : 1;
    RWBit rw_bit : 1;
    u8 dc_bit : 1;
    ExecutableBit executable_bit : 1;
    DescriptorTypeBit type_bit : 1;
    DescriptorPrivilegeLevel dpl_bit : 2;
    PresentBit present_bit : 1;
};
static_assert(sizeof(AccessByte) == 1);

struct PACK SystemSegmentAccessByte {
    enum class DescriptorTypeBit : u8 {
        kSystemSegment = 0,
        kUsualSegment  = 1,
    };

    enum class DescriptorPrivilegeLevel : u8 {
        kKernel = 0,
        kRing1  = 1,
        kRing2  = 2,
        kUser   = 3,
    };

    enum class PresentBit : u8 {
        kPresent    = 1,
        kNotPresent = 0,
    };

    enum class ProtectedModeTypes : u8 {
        kTSS16Bit     = 1,
        kLDT          = 2,
        kTSS16BitBusy = 3,
        kTSS32Bit     = 9,
        kTSS32BitBusy = 11,
    };

    enum class LongModeTypes : u8 {
        kLDT     = 2,
        kTSS     = 9,
        kTSSBusy = 11,
    };

    u8 type : 4;
    DescriptorTypeBit type_bit : 1;
    DescriptorPrivilegeLevel dpl_bits : 2;
    PresentBit present_bit : 1;
};
static_assert(sizeof(AccessByte) == 1);

template <class AccessByteT = AccessByte>
struct PACK GdtEntry {
    enum class LongModeFlag : u8 {
        kLongModeDisabled = 0,
        kLongModeEnabled  = 1,
    };

    enum class SizeFlag : u8 {
        k16BitMode = 0,
        k32BitMode = 1,
    };

    enum class GranularityFlag : u8 {
        kByteGranularity = 0,
        kPageGranularity = 1,
    };

    struct PACK Flags {
        u8 reserved : 1;
        LongModeFlag long_mode_flag : 1;
        SizeFlag size_flag : 1;
        GranularityFlag granularity_flag : 1;
    };

    u16 limit;
    u16 base;
    u8 base_mid;
    AccessByteT access;
    u8 limit_high : 4;
    u8 reserved : 1;
    LongModeFlag long_mode_flag : 1;
    SizeFlag size_flag : 1;
    GranularityFlag granularity_flag : 1;
    u8 base_high;
};
static_assert(sizeof(GdtEntry<>) == 8);
static_assert(sizeof(GdtEntry<SystemSegmentAccessByte>) == 8);

struct PACK GdtSystemSegmentDescriptor {
    GdtEntry<SystemSegmentAccessByte> low;
    u32 base_upper32;
    u32 reserved;
};
static_assert(sizeof(GdtSystemSegmentDescriptor) == 16);

struct PACK Gdtr {
    u16 limit;
    u64 base;
};

struct alignas(16) PACK GDT {
    static constexpr u16 kKernelCodeSelector = 0x08;
    static constexpr u16 kKernelDataSelector = 0x10;
    static constexpr u16 kUserCodeSelector   = 0x18;
    static constexpr u16 kUserDataSelector   = 0x20;
    static constexpr u16 kTssSelector        = 0x28;

    GdtEntry<> null_entry;
    GdtEntry<> kernel_code;
    GdtEntry<> kernel_data;
    GdtEntry<> user_code;
    GdtEntry<> user_data;
    GdtSystemSegmentDescriptor tss_descriptor;
};

void DefaultEntryInit(GdtEntry<> &entry, AccessByte access, GdtEntry<>::Flags flags);
void DefaultTssEntryInit(GdtSystemSegmentDescriptor &entry, u64 tss_address);
void DefaultGdtInit(GDT &gdt, u64 tss_address);

}  // namespace cpu

#endif  // KERNEL_ARCH_X86_64_SRC_CPU_GDT_HPP_
