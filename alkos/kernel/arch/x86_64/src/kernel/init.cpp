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
#include <terminal.hpp>

#include "abi/boot_params.hpp"
#include "cpu/utils.hpp"
#include "panic.hpp"

//==============================================================================
// External Functions and Variables
//==============================================================================

extern "C" void EnableOSXSave();
extern "C" void EnableSSE();
extern "C" void EnableAVX();
extern "C" void EnterKernel(u64 kernel_entry_addr);

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

    TRACE_INFO("CPU Model: %d / %08X", GetCpuModel(), GetCpuModel());

    if (kernel_init_params == nullptr) {
        arch::KernelPanic("KernelInitialParams is null!");
    }

    TRACE_INFO("Setting up CPU features...");
    BlockHardwareInterrupts();

    /* NOTE: sequence is important */
    EnableOSXSave();
    EnableSSE();
    EnableAVX();

    HardwareModule::Init();
    HardwareModule::Get().GetInterrupts().FirstStageInit();

    EnableHardwareInterrupts();

    TRACE_INFO("Leaving PreKernelInit and entering kernel...");
}
