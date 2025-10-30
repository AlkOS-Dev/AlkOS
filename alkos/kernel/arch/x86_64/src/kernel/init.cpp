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

#include <hal/impl/kernel.hpp>
#include <hal/panic.hpp>
#include <hal/terminal.hpp>
#include <hardware/cores.hpp>

#include "abi/boot_args.hpp"
#include "cpu/utils.hpp"

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

static hardware::CoreLocal g_CoreLocal{};

namespace arch
{
void ArchInit(const RawBootArguments &)
{
    TRACE_INFO("In ArchInit...");
    TRACE_INFO("CPU Model: %d / %08X", GetCpuModel(), GetCpuModel());

    SetCoreLocalData(&g_CoreLocal);

    TRACE_INFO("Setting up CPU features...");
    BlockHardwareInterrupts();

    /* NOTE: sequence is important */
    EnableOSXSave();
    EnableSSE();
    EnableAVX();

    HardwareModule::Init();
    HardwareModule::Get().GetInterrupts().FirstStageInit();

    EnableHardwareInterrupts();

    TRACE_INFO("Leaving ArchInit");
}
}  // namespace arch
