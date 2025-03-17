#include <string.h>
#include <arch_utils.hpp>
#include <cpu/cpu_info.hpp>

namespace
{
CPUInfo cpuInfo{};
}

const CPUInfo& GetCPUInfo() { return cpuInfo; }

void InitCPUInfo()
{
    u32 regs[4];

    // Get vendor string and max CPUID level
    CPUID(0, regs);
    char vendorString[13] = {0};
    cpuInfo.max_cpuid     = regs[0];
    memcpy(&vendorString[0], &regs[1], 4);
    memcpy(&vendorString[4], &regs[3], 4);
    memcpy(&vendorString[8], &regs[2], 4);

    cpuInfo.vendor = CPUVendor::Unknown;
    if (strcmp(vendorString, "GenuineIntel") == 0) {
        cpuInfo.vendor = CPUVendor::Intel;
    } else if (strcmp(vendorString, "AuthenticAMD") == 0) {
        cpuInfo.vendor = CPUVendor::AMD;
    }

    // Get CPU family, model and brand string
    CPUID(1, regs);
    cpuInfo.stepping = regs[0] & 0xF;

    byte base_family = (regs[0] >> 8) & 0xF;
    if (base_family == 0xF) {
        base_family += (regs[0] >> 20) & 0xFF;
    }
    cpuInfo.family = base_family;

    byte base_model = (regs[0] >> 4) & 0xF;
    if (base_family == 0x6 || base_family == 0xF) {
        base_model += ((regs[0] >> 16) & 0xF) << 4;
    }
    cpuInfo.model = base_model;

    // Set basic features
    cpuInfo.features[0] = regs[3];
    cpuInfo.features[1] = regs[2];

    // Get core count and initial APIC ID
    CPUID(0x0B, 0, regs);
    cpuInfo.threads_per_core = regs[1] & 0xFFFF;
    CPUID(0x0B, 1, regs);
    cpuInfo.core_count = (regs[1] & 0xFFFF) / cpuInfo.threads_per_core;
}
