#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__x86_64__)
#error "AlkOS needs to be compiled with a x86_64-elf compiler"
#endif

#include <cpuid.h>
#include <extensions/debug.hpp>
#include <extensions/expected.hpp>
#include <extensions/internal/formats.hpp>

#include <modules/hardware.hpp>
#include <modules/memory.hpp>
#include <terminal.hpp>

#include "abi/boot_params.hpp"
#include "cpu/utils.hpp"
#include "panic.hpp"

//==============================================================================
// External Functions and Variables
//==============================================================================

BEGIN_DECL_C

void EnableOSXSave();
void EnableSSE();
void EnableAVX();

END_DECL_C

static int GetCpuModel()
{
    int ebx, unused;
    __cpuid(0, unused, ebx, unused, unused);
    return ebx;
}

//==================================================================================
// Main Entry Point
//==================================================================================

extern "C" void PreKernelInit(KernelInitialParams* kernel_init_params)
{
    arch::TerminalInit();
    TRACE_INFO("In PreKernelInit...");

    ASSERT_NOT_NULL(kernel_init_params, "Kernel reached with no initial params from loader");

    TRACE_INFO("CPU Model: %d / %08X", GetCpuModel(), GetCpuModel());

    TRACE_INFO("Setting up CPU features...");
    BlockHardwareInterrupts();

    /* NOTE: sequence is important */
    EnableOSXSave();
    EnableSSE();
    EnableAVX();

    HardwareModule::Init();
    HardwareModule::Get().GetInterrupts().FirstStageInit();

    arch::PmmConfig b_pmm_conf;
    b_pmm_conf.pmm_bitmap_addr = PhysicalPtr<void>(kernel_init_params->mem_info_bitmap_addr);
    b_pmm_conf.pmm_total_pages = kernel_init_params->mem_info_total_pages;

    arch::VmmConfig vmm_conf;
    vmm_conf.pml4_table = PhysicalPtr<PageMapTable<4>>(kernel_init_params->pml_4_table_phys_addr);

    MemoryModule::Init();
    MemoryModule::Get().GetPmm().Configure(b_pmm_conf);
    MemoryModule::Get().GetVmm().Configure(vmm_conf);

    EnableHardwareInterrupts();

    TRACE_INFO("Leaving PreKernelInit and entering kernel...");
}
