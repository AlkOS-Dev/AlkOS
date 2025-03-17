#ifndef ARCH_X86_64_KERNEL_CPU_CPU_INFO_HPP_
#define ARCH_X86_64_KERNEL_CPU_CPU_INFO_HPP_

#include <cpu/features_flags.hpp>
#include <defines.hpp>
#include <extensions/types.hpp>

static constexpr u16 kMaxCores = 256;

enum class CPUVendor {
    Unknown,
    Intel,
    AMD,
};

struct CoreInfo {
    u16 lapic_id;
} PACK;

struct CPUInfo {
    CPUVendor vendor;
    u8 stepping;
    u8 family;
    u8 model;
    u32 features[kFeaturesDwords];
    u32 max_cpuid;
    u16 core_count;
    u16 threads_per_core;
    CoreInfo cores[kMaxCores];

    [[nodiscard]] bool HasFeature(u16 feature) const
    {
        return features[feature / 32] & (1 << (feature % 32));
    }
} PACK;

[[nodiscard]] const CPUInfo &GetCPUInfo();

void InitCPUInfo();

#endif  // ARCH_X86_64_KERNEL_CPU_CPU_INFO_HPP_
