#ifndef ARCH_X86_64_KERNEL_CPU_FEATURES_FLAGS_HPP_
#define ARCH_X86_64_KERNEL_CPU_FEATURES_FLAGS_HPP_

#include <extensions/types.hpp>

/**
 * @file features_flags.hpp
 * @brief Defines CPU feature flags for x86_64 architecture
 *
 * Proides definitions for CPU feature flags for x86_64 architecture.
 */

/* Number of dwords worth of cpu info */
static constexpr u8 kFeaturesDwords = 16;

/* Intel-defined CPU features, CPUID level 0x00000001 (edx), dword 0 */
static constexpr u16 X86_FEATURE_FPU     = (0 * 32 + 0);   // Onboard FPU
static constexpr u16 X86_FEATURE_VME     = (0 * 32 + 1);   // Virtual Mode Extensions
static constexpr u16 X86_FEATURE_DE      = (0 * 32 + 2);   // Debugging Extensions
static constexpr u16 X86_FEATURE_PSE     = (0 * 32 + 3);   // Page Size Extensions
static constexpr u16 X86_FEATURE_TSC     = (0 * 32 + 4);   // Time Stamp Counter
static constexpr u16 X86_FEATURE_MSR     = (0 * 32 + 5);   // Model-Specific Registers
static constexpr u16 X86_FEATURE_PAE     = (0 * 32 + 6);   // Physical Address Extensions
static constexpr u16 X86_FEATURE_MCE     = (0 * 32 + 7);   // Machine Check Exception
static constexpr u16 X86_FEATURE_CX8     = (0 * 32 + 8);   // CMPXCHG8 instruction
static constexpr u16 X86_FEATURE_APIC    = (0 * 32 + 9);   // Onboard APIC
static constexpr u16 X86_FEATURE_SEP     = (0 * 32 + 11);  // SYSENTER/SYSEXIT
static constexpr u16 X86_FEATURE_MTRR    = (0 * 32 + 12);  // Memory Type Range Registers
static constexpr u16 X86_FEATURE_PGE     = (0 * 32 + 13);  // Page Global Enable
static constexpr u16 X86_FEATURE_MCA     = (0 * 32 + 14);  // Machine Check Architecture
static constexpr u16 X86_FEATURE_CMOV    = (0 * 32 + 15);  // CMOV instructions
static constexpr u16 X86_FEATURE_PAT     = (0 * 32 + 16);  // Page Attribute Table
static constexpr u16 X86_FEATURE_PSE36   = (0 * 32 + 17);  // 36-bit PSEs
static constexpr u16 X86_FEATURE_PN      = (0 * 32 + 18);  // Processor serial number
static constexpr u16 X86_FEATURE_CLFLUSH = (0 * 32 + 19);  // CLFLUSH instruction
static constexpr u16 X86_FEATURE_DS      = (0 * 32 + 21);  // Debug Store
static constexpr u16 X86_FEATURE_ACPI    = (0 * 32 + 22);  // ACPI via MSR
static constexpr u16 X86_FEATURE_MMX     = (0 * 32 + 23);  // Multimedia Extensions
static constexpr u16 X86_FEATURE_FXSR    = (0 * 32 + 24);  // FXSAVE/FXRSTOR, CR4.OSFXSR
static constexpr u16 X86_FEATURE_SSE     = (0 * 32 + 25);  // SSE
static constexpr u16 X86_FEATURE_SSE2    = (0 * 32 + 26);  // SSE2
static constexpr u16 X86_FEATURE_SS      = (0 * 32 + 27);  // CPU self snoop
static constexpr u16 X86_FEATURE_HT      = (0 * 32 + 28);  // Hyper-Threading
static constexpr u16 X86_FEATURE_ACC     = (0 * 32 + 29);  // "tm" Automatic clock control
static constexpr u16 X86_FEATURE_IA64    = (0 * 32 + 30);  // IA-64 processor
static constexpr u16 X86_FEATURE_PBE     = (0 * 32 + 31);  // Pending Break Enable

/* Intel-defined CPU features, CPUID level 0x00000001 (ecx), dword 1 */
static constexpr u16 X86_FEATURE_SEE3      = (1 * 32 + 0);   // SSE3
static constexpr u16 X86_FEATURE_PCLMULQDQ = (1 * 32 + 1);   // PCLMULQDQ instruction
static constexpr u16 X86_FEATURE_DTES64    = (1 * 32 + 2);   // 64-bit Debug Store
static constexpr u16 X86_FEATURE_MWAIT     = (1 * 32 + 3);   // Monitor/Mwait support ("monitor")
static constexpr u16 X86_FEATURE_DSCPL     = (1 * 32 + 4);   // CPL Qual. Debug Store ("ds_cpl")
static constexpr u16 X86_FEATURE_VMX       = (1 * 32 + 5);   // Hardware virtualization
static constexpr u16 X86_FEATURE_SMX       = (1 * 32 + 6);   // Safer mode
static constexpr u16 X86_FEATURE_EST       = (1 * 32 + 7);   // Enhanced SpeedStep
static constexpr u16 X86_FEATURE_TM2       = (1 * 32 + 8);   // Thermal Monitor 2
static constexpr u16 X86_FEATURE_SSSE3     = (1 * 32 + 9);   // Supplemental SSE3
static constexpr u16 X86_FEATURE_CID       = (1 * 32 + 10);  // Context ID
static constexpr u16 X86_FEATURE_SDBG      = (1 * 32 + 11);  // Silicon Debug
static constexpr u16 X86_FEATURE_FMA       = (1 * 32 + 12);  // Fused multiply-add
static constexpr u16 X86_FEATURE_CX16      = (1 * 32 + 13);  // CMPXCHG16B
static constexpr u16 X86_FEATURE_XTPR      = (1 * 32 + 14);  // Send Task Priority Messages
static constexpr u16 X86_FEATURE_PDCM      = (1 * 32 + 15);  // Performance Capabilities
static constexpr u16 X86_FEATURE_PCID      = (1 * 32 + 17);  // Process Context Identifiers
static constexpr u16 X86_FEATURE_DCA       = (1 * 32 + 18);  // Direct Cache Access
static constexpr u16 X86_FEATURE_SSE4_1    = (1 * 32 + 19);  // SSE4.1 ("sse4_1")
static constexpr u16 X86_FEATURE_SSE4_2    = (1 * 32 + 20);  // SSE4.2 ("sse4_2")
static constexpr u16 X86_FEATURE_X2APIC    = (1 * 32 + 21);  // x2APIC
static constexpr u16 X86_FEATURE_MOVBE     = (1 * 32 + 22);  // MOVBE instruction
static constexpr u16 X86_FEATURE_POPCNT    = (1 * 32 + 23);  // POPCNT instruction
static constexpr u16 X86_FEATURE_TSC_DEADLINE_TIMER = (1 * 32 + 24);  // TSC deadline timer
static constexpr u16 X86_FEATURE_AES                = (1 * 32 + 25);  // AES instructions
static constexpr u16 X86_FEATURE_XSAVE              = (1 * 32 + 26);  // XSAVE/XRSTOR/XSETBV/XGETBV
static constexpr u16 X86_FEATURE_OSXSAVE            = (1 * 32 + 27);  // XSAVE enabled in the OS
static constexpr u16 X86_FEATURE_AVX                = (1 * 32 + 28);  // Advanced Vector Extensions
static constexpr u16 X86_FEATURE_F16C               = (1 * 32 + 29);  // 16-bit fp conversions
static constexpr u16 X86_FEATURE_RDRAND             = (1 * 32 + 30);  // The RDRAND instruction
static constexpr u16 X86_FEATURE_HYPERVISOR         = (1 * 32 + 31);  // Running on a hypervisor

/* AMD-defined CPU features, CPUID level 0x80000001, dword 2 */
static constexpr u16 X86_FEATURE_SYSCALL  = (2 * 32 + 11);  // SYSCALL/SYSRET
static constexpr u16 X86_FEATURE_MP       = (2 * 32 + 19);  // MP Capable.
static constexpr u16 X86_FEATURE_NX       = (2 * 32 + 20);  // Execute Disable
static constexpr u16 X86_FEATURE_MMXEXT   = (2 * 32 + 22);  // AMD MMX extensions
static constexpr u16 X86_FEATURE_FXSR_OPT = (2 * 32 + 25);  // FXSAVE/FXRSTOR optimizations
static constexpr u16 X86_FEATURE_GBPAGES  = (2 * 32 + 26);  //  GB pages
static constexpr u16 X86_FEATURE_RDTSCP   = (2 * 32 + 27);  // RDTSCP
static constexpr u16 X86_FEATURE_LM       = (2 * 32 + 29);  // Long Mode (x86-64)
static constexpr u16 X86_FEATURE_3DNOWEXT = (2 * 32 + 30);  // AMD 3DNow! extensions
static constexpr u16 X86_FEATURE_3DNOW    = (2 * 32 + 31);  // 3DNow!

/* More extended AMD flags: CPUID level 0x80000001, ecx, dword 3 */
static constexpr u16 X86_FEATURE_LAHF_LM       = (3 * 32 + 0);   // LAHF/SAHF in long mode
static constexpr u16 X86_FEATURE_CMP_LEGACY    = (3 * 32 + 1);   // HyperThreading not valid if yes
static constexpr u16 X86_FEATURE_SVM           = (3 * 32 + 2);   // Secure virtual machine
static constexpr u16 X86_FEATURE_EXTAPIC       = (3 * 32 + 3);   // Extended APIC space
static constexpr u16 X86_FEATURE_CR8_LEGACY    = (3 * 32 + 4);   // CR8 in 32-bit mode
static constexpr u16 X86_FEATURE_ABM           = (3 * 32 + 5);   // Advanced bit manipulation
static constexpr u16 X86_FEATURE_SSE4A         = (3 * 32 + 6);   // SSE4A
static constexpr u16 X86_FEATURE_MISALIGNSSE   = (3 * 32 + 7);   // Misaligned SSE mode
static constexpr u16 X86_FEATURE_3DNOWPREFETCH = (3 * 32 + 8);   // 3DNow prefetch instructions
static constexpr u16 X86_FEATURE_OSVW          = (3 * 32 + 9);   // OS Visible Workaround
static constexpr u16 X86_FEATURE_IBS           = (3 * 32 + 10);  // Instruction Based Sampling
static constexpr u16 X86_FEATURE_XOP           = (3 * 32 + 11);  // extended AVX instructions
static constexpr u16 X86_FEATURE_SKINIT        = (3 * 32 + 12);  // SKINIT/STGI instructions
static constexpr u16 X86_FEATURE_WDT           = (3 * 32 + 13);  // Watchdog timer
static constexpr u16 X86_FEATURE_LWP           = (3 * 32 + 15);  // Light Weight Profiling
static constexpr u16 X86_FEATURE_FMA4          = (3 * 32 + 16);  // 4 operand MAC instructions
static constexpr u16 X86_FEATURE_TCE           = (3 * 32 + 17);  // translation cache extension
static constexpr u16 X86_FEATURE_NODEID_MSR    = (3 * 32 + 19);  // NodeId MSR
static constexpr u16 X86_FEATURE_TBM           = (3 * 32 + 21);  // Trailing bit manipulations
static constexpr u16 X86_FEATURE_TOPOEXT       = (3 * 32 + 22);  // Topology extensions CPUID leafs
static constexpr u16 X86_FEATURE_PERFCTR_CORE =
    (3 * 32 + 23);                                            // Core performance counter extensions
static constexpr u16 X86_FEATURE_PERFCTR_NB = (3 * 32 + 24);  // NB performance counter extensions
static constexpr u16 X86_FEATURE_BPEXT      = (3 * 32 + 26);  // data breakpoint extension
static constexpr u16 X86_FEATURE_PTSC       = (3 * 32 + 27);  // performance time-stamp counter
static constexpr u16 X86_FEATURE_PERFCTR_L2 = (3 * 32 + 28);  // L2 performance counter extensions
static constexpr u16 X86_FEATURE_MWAITX     = (3 * 32 + 29);  // MWAIT extension (MONITORX/MWAITX)

/* Other features, Linux-defined mapping, dword 4 */
static constexpr u16 X86_FEATURE_CXMMX        = (4 * 32 + 0);  // Cyrix MMX extensions
static constexpr u16 X86_FEATURE_K6_MTRR      = (4 * 32 + 1);  // AMD K6 nonstandard MTRRs
static constexpr u16 X86_FEATURE_CYRIX_ARR    = (4 * 32 + 2);  // Cyrix ARRs (= MTRRs)
static constexpr u16 X86_FEATURE_CENTAUR_MCR  = (4 * 32 + 3);  // Centaur MCRs (= MTRRs)
static constexpr u16 X86_FEATURE_K8           = (4 * 32 + 4);  // Opteron, Athlon64
static constexpr u16 X86_FEATURE_K7           = (4 * 32 + 5);  // Athlon
static constexpr u16 X86_FEATURE_P3           = (4 * 32 + 6);  // P3
static constexpr u16 X86_FEATURE_P4           = (4 * 32 + 7);  // P4
static constexpr u16 X86_FEATURE_CONSTANT_TSC = (4 * 32 + 8);  // TSC ticks at a constant rate
static constexpr u16 X86_FEATURE_UP           = (4 * 32 + 9);  // SMP kernel running on up
static constexpr u16 X86_FEATURE_ART = (4 * 32 + 10);  // Platform has always running timer (ART)
static constexpr u16 X86_FEATURE_ARCH_PERFMON = (4 * 32 + 11);  // Intel Architectural PerfMon
static constexpr u16 X86_FEATURE_PEBS         = (4 * 32 + 12);  // Precise-Event Based Sampling
static constexpr u16 X86_FEATURE_BTS          = (4 * 32 + 13);  // Branch Trace Store
static constexpr u16 X86_FEATURE_SYSCALL32    = (4 * 32 + 14);  // Syscall in IA-32 userspace
static constexpr u16 X86_FEATURE_SYSENTER32   = (4 * 32 + 15);  // Sysenter in IA-32 userspace
static constexpr u16 X86_FEATURE_REP_GOOD     = (4 * 32 + 16);  // REP microcode works well
static constexpr u16 X86_FEATURE_MFENCE_RDTSC = (4 * 32 + 17);  // Mfence synchronizes RDTSC
static constexpr u16 X86_FEATURE_LFENCE_RDTSC = (4 * 32 + 18);  // Lfence synchronizes RDTSC
static constexpr u16 X86_FEATURE_ACC_POWER    = (4 * 32 + 19);  // AMD Accumulated Power Mechanism
static constexpr u16 X86_FEATURE_NOPL         = (4 * 32 + 20);  // The NOPL (0F 1F) instructions
static constexpr u16 X86_FEATURE_ALWAYS       = (4 * 32 + 21);  // Always-present feature
static constexpr u16 X86_FEATURE_XTOPOLOGY    = (4 * 32 + 22);  // CPU topology enum extensions
static constexpr u16 X86_FEATURE_TSC_RELIABLE = (4 * 32 + 23);  // TSC is known to be reliable
static constexpr u16 X86_FEATURE_NONSTOP_TSC  = (4 * 32 + 24);  // TSC does not stop in C states
static constexpr u16 X86_FEATURE_CLFLUSH_MONITOR =
    (4 * 32 + 25);  // CLFLUSH monitors if a cache line is dirty
static constexpr u16 X86_FEATURE_EXTD_APICID    = (4 * 32 + 26);  // Has extended APICID (8 bits)
static constexpr u16 X86_FEATURE_AMD_DCM        = (4 * 32 + 27);  // Multi-node processor
static constexpr u16 X86_FEATURE_APERFMPERF     = (4 * 32 + 28);  // APERF/MPERF Performance MSRs
static constexpr u16 X86_FEATURE_EAGER_FPU      = (4 * 32 + 29);  // Non lazy FPU restore
static constexpr u16 X86_FEATURE_NONSTOP_TSC_S3 = (4 * 32 + 30);  // TSC doesn't stop in S3 state

/* Auxiliary flags: Linux defined - For features scattered in various CPUID levels, dword 5. */
static constexpr u16 X86_FEATURE_CPB = (5 * 32 + 2);  // AMD Core Performance Boost
static constexpr u16 X86_FEATURE_EPB = (5 * 32 + 3);  // IA32_ENERGY_PERF_BIAS support
static constexpr u16 X86_FEATURE_INVPCID_SINGLE =
    (5 * 32 + 4);  // Effectively INVPCID && CR4.PCIDE=1
static constexpr u16 X86_FEATURE_HW_PSTATE     = (5 * 32 + 8);  // AMD HW-PState
static constexpr u16 X86_FEATURE_PROC_FEEDBACK = (5 * 32 + 9);  // AMD ProcFeedbackInterface
static constexpr u16 X86_FEATURE_RETPOLINE =
    (5 * 32 + 12);  // Generic Retpoline mitigation for Spectre v2
static constexpr u16 X86_FEATURE_RETPOLINE_AMD =
    (5 * 32 + 13);  // AMD Retpoline mitigation for Spectre v2
static constexpr u16 X86_FEATURE_AVX512_4VNNIW =
    (5 * 32 + 16);  // AVX-512 Neural Network Instructions
static constexpr u16 X86_FEATURE_AVX512_4FMAPS =
    (5 * 32 + 17);  // AVX-512 Multiply Accumulation Single precision
static constexpr u16 X86_FEATURE_RSB_CTXSW = (5 * 32 + 19);  // Fill RSB on context switches
static constexpr u16 X86_FEATURE_KAISER =
    (5 * 32 + 31);  // CONFIG_PAGE_TABLE_ISOLATION w\/o nokaiser

/* Virtualization flags: Linux defined, dword 6 */
static constexpr u16 X86_FEATURE_TPR_SHADOW   = (6 * 32 + 0);   // Intel TPR Shadow
static constexpr u16 X86_FEATURE_VNMI         = (6 * 32 + 1);   // Intel Virtual NMI
static constexpr u16 X86_FEATURE_FLEXPRIORITY = (6 * 32 + 2);   // Intel FlexPriority
static constexpr u16 X86_FEATURE_EPT          = (6 * 32 + 3);   // Intel Extended Page Table
static constexpr u16 X86_FEATURE_VPID         = (6 * 32 + 4);   // Intel Virtual Processor ID
static constexpr u16 X86_FEATURE_VMMCALL      = (6 * 32 + 15);  // Prefer vmmcall to vmcall
static constexpr u16 X86_FEATURE_XENPV        = (6 * 32 + 16);  // Xen paravirtual guest

/* Intel-defined CPU features, CPUID level 0x00000007:0 (ebx), dword 7 */
static constexpr u16 X86_FEATURE_FSGSBASE   = (7 * 32 + 0);  // {RD/WR}{FS/GS}BASE instructions
static constexpr u16 X86_FEATURE_TSC_ADJUST = (7 * 32 + 1);  // TSC adjustment MSR 0x3b
static constexpr u16 X86_FEATURE_BMI1     = (7 * 32 + 3);   // 1st group bit manipulation extensions
static constexpr u16 X86_FEATURE_HLE      = (7 * 32 + 4);   // Hardware Lock Elision
static constexpr u16 X86_FEATURE_AVX2     = (7 * 32 + 5);   // AVX2 instructions
static constexpr u16 X86_FEATURE_SMEP     = (7 * 32 + 7);   // Supervisor Mode Execution Protection
static constexpr u16 X86_FEATURE_BMI2     = (7 * 32 + 8);   // 2nd group bit manipulation extensions
static constexpr u16 X86_FEATURE_ERMS     = (7 * 32 + 9);   // Enhanced REP MOVSB/STOSB
static constexpr u16 X86_FEATURE_INVPCID  = (7 * 32 + 10);  // Invalidate Processor Context ID
static constexpr u16 X86_FEATURE_RTM      = (7 * 32 + 11);  // Restricted Transactional Memory
static constexpr u16 X86_FEATURE_CQM      = (7 * 32 + 12);  // Cache QoS Monitoring
static constexpr u16 X86_FEATURE_MPX      = (7 * 32 + 14);  // Memory Protection Extension
static constexpr u16 X86_FEATURE_AVX512F  = (7 * 32 + 16);  // AVX-512 Foundation
static constexpr u16 X86_FEATURE_AVX512DQ = (7 * 32 + 17);  // AVX-512 DQ Instructions
static constexpr u16 X86_FEATURE_RDSEED   = (7 * 32 + 18);  // The RDSEED instruction
static constexpr u16 X86_FEATURE_ADX      = (7 * 32 + 19);  // The ADCX and ADOX instructions
static constexpr u16 X86_FEATURE_SMAP     = (7 * 32 + 20);  // Supervisor Mode Access Prevention
static constexpr u16 X86_FEATURE_CLFLUSHOPT = (7 * 32 + 23);  // CLFLUSHOPT instruction
static constexpr u16 X86_FEATURE_CLWB       = (7 * 32 + 24);  // CLWB instruction
static constexpr u16 X86_FEATURE_INTEL_PT   = (7 * 32 + 25);  // Intel Processor Trace
static constexpr u16 X86_FEATURE_AVX512PF   = (7 * 32 + 26);  // AVX-512 Prefetch
static constexpr u16 X86_FEATURE_AVX512ER   = (7 * 32 + 27);  // AVX-512 Exponential and Reciprocal
static constexpr u16 X86_FEATURE_AVX512CD   = (7 * 32 + 28);  // AVX-512 Conflict Detection
static constexpr u16 X86_FEATURE_SHA_NI     = (7 * 32 + 29);  // SHA1/SHA256 Instruction Extensions
static constexpr u16 X86_FEATURE_AVX512BW   = (7 * 32 + 30);  // AVX-512 BW Instructions
static constexpr u16 X86_FEATURE_AVX512VL   = (7 * 32 + 31);  // AVX-512 VL Extensions

/* Extended state features, CPUID level 0x0000000d:1 (eax), dword 8 */
static constexpr u16 X86_FEATURE_XSAVEOPT = (8 * 32 + 0);  // XSAVEOPT
static constexpr u16 X86_FEATURE_XSAVEC   = (8 * 32 + 1);  // XSAVEC
static constexpr u16 X86_FEATURE_XGETBV1  = (8 * 32 + 2);  // XGETBV with ECX = 1
static constexpr u16 X86_FEATURE_XSAVES   = (8 * 32 + 3);  // XSAVES/XRSTORS

/* Intel-defined CPU QoS Sub-leaf, CPUID level 0x0000000F:0 (edx), dword 9 */
static constexpr u16 X86_FEATURE_CQM_LLC = (9 * 32 + 1);  // LLC QoS if 1

/* Intel-defined CPU QoS Sub-leaf, CPUID level 0x0000000F:1 (edx), word 10 */
static constexpr u16 X86_FEATURE_CQM_OCCUP_LLC = (10 * 32 + 0);  // LLC occupancy monitoring if 1
static constexpr u16 X86_FEATURE_CQM_MBM_TOTAL = (10 * 32 + 1);  // LLC Total MBM monitoring
static constexpr u16 X86_FEATURE_CQM_MBM_LOCAL = (10 * 32 + 2);  // LLC Local MBM monitoring

/* AMD-defined CPU features, CPUID level 0x80000008 (ebx), dword 11 */
static constexpr u16 X86_FEATURE_CLZERO = (11 * 32 + 0);  // CLZERO instruction
static constexpr u16 X86_FEATURE_IRPERF = (11 * 32 + 1);  // Instructions Retired Count

/* Thermal and Power Management Leaf, CPUID level 0x00000006 (eax), dword 12 */
static constexpr u16 X86_FEATURE_DTHERM         = (12 * 32 + 0);   // Digital Thermal Sensor
static constexpr u16 X86_FEATURE_IDA            = (12 * 32 + 1);   // Intel Dynamic Acceleration
static constexpr u16 X86_FEATURE_ARAT           = (12 * 32 + 2);   // Always Running APIC Timer
static constexpr u16 X86_FEATURE_PLN            = (12 * 32 + 4);   // Intel Power Limit Notification
static constexpr u16 X86_FEATURE_PTS            = (12 * 32 + 6);   // Intel Package Thermal Status
static constexpr u16 X86_FEATURE_HWP            = (12 * 32 + 7);   // Intel Hardware P-states
static constexpr u16 X86_FEATURE_HWP_NOTIFY     = (12 * 32 + 8);   // HWP Notification
static constexpr u16 X86_FEATURE_HWP_ACT_WINDOW = (12 * 32 + 9);   // HWP Activity Window
static constexpr u16 X86_FEATURE_HWP_EPP        = (12 * 32 + 10);  // HWP Energy Perf. Preference
static constexpr u16 X86_FEATURE_HWP_PKG_REQ    = (12 * 32 + 11);  // HWP Package Level Request

/* AMD SVM Feature Identification, CPUID level 0x8000000a (edx), dword 13 */
static constexpr u16 X86_FEATURE_NPT        = (13 * 32 + 0);  // Nested Page Table support
static constexpr u16 X86_FEATURE_LBRV       = (13 * 32 + 1);  // LBR Virtualization support
static constexpr u16 X86_FEATURE_SVML       = (13 * 32 + 2);  // SVM locking MSR ("svm_lock")
static constexpr u16 X86_FEATURE_NRIPS      = (13 * 32 + 3);  // SVM next_rip save ("nrip_save")
static constexpr u16 X86_FEATURE_TSCRATEMSR = (13 * 32 + 4);  // TSC scaling support ("tsc_scale")
static constexpr u16 X86_FEATURE_VMCBCLEAN =
    (13 * 32 + 5);  // VMCB clean bits support ("vmcb_clean")
static constexpr u16 X86_FEATURE_FLUSHBYASID   = (13 * 32 + 6);   // Flush-by-ASID support
static constexpr u16 X86_FEATURE_DECODEASSISTS = (13 * 32 + 7);   // Decode Assists support
static constexpr u16 X86_FEATURE_PAUSEFILTER   = (13 * 32 + 10);  // Filtered pause intercept
static constexpr u16 X86_FEATURE_PFTHRESHOLD   = (13 * 32 + 12);  // Pause filter threshold
static constexpr u16 X86_FEATURE_AVIC          = (13 * 32 + 13);  // Virtual Interrupt Controller

/* Intel-defined CPU features, CPUID level 0x00000007:0 (ecx), dword 14 */
static constexpr u16 X86_FEATURE_PKU   = (14 * 32 + 3);  // Protection Keys for Userspace
static constexpr u16 X86_FEATURE_OSPKE = (14 * 32 + 4);  // OS Protection Keys Enable

/* AMD-defined CPU features, CPUID level 0x80000007 (ebx), dword 15 */
static constexpr u16 X86_FEATURE_OVERFLOW_RECOV = (15 * 32 + 0);  // MCA overflow recovery support
static constexpr u16 X86_FEATURE_SUCCOR =
    (15 * 32 + 1);  // Uncorrectable error containment and recovery
static constexpr u16 X86_FEATURE_SMCA = (15 * 32 + 3);  // Scalable MCA

#endif  // ARCH_X86_64_KERNEL_CPU_FEATURES_FLAGS_HPP_
