#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__x86_64__)
#error "AlkOS needs to be compiled with a x86_64-elf compiler"
#endif

/* internal includes */
#include <cpuid.h>
#include <extensions/debug.hpp>
#include <extensions/internal/formats.hpp>
#include <modules/hardware.hpp>
#include <terminal.hpp>
#include "cpu/utils.hpp"

/* external init procedures */
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

extern "C" void PreKernelInit(void* loader_data)
{
    TODO_WHEN_DEBUGGING_FRAMEWORK

    arch::TerminalInit();
    TRACE_INFO("In 64 bit mode");

    TRACE_INFO("CPU Model: %d / %08X", GetCpuModel(), GetCpuModel());

    TRACE_INFO("Checking for LoaderData...");
    if (loader_data == nullptr) {
        TRACE_ERROR("LoaderData check failed!");
        OsHangNoInterrupts();
    }
    TRACE_SUCCESS("LoaderData found!");
    TODO_WHEN_DEBUGGING_FRAMEWORK
    TRACE_INFO("Starting pre-kernel initialization");

    TRACE_INFO("Starting to setup CPU features");
    BlockHardwareInterrupts();

    /* NOTE: sequence is important */
    TRACE_INFO("Setting up OS XSAVE...");
    EnableOSXSave();
    TRACE_SUCCESS("OS XSAVE setup complete!");

    TRACE_INFO("Setting up SSE...");
    EnableSSE();
    TRACE_SUCCESS("SSE setup complete!");

    TRACE_INFO("Setting up AVX...");
    EnableAVX();
    TRACE_SUCCESS("AVX setup complete!");

    // HardwareModule::Init();
    // HardwareModule::Get().GetInterrupts().FirstStageInit();
    TRACE_INFO("KERNEL END");
    OsHangNoInterrupts();

    EnableHardwareInterrupts();
    TRACE_INFO("Finished cpu features setup.");
}
