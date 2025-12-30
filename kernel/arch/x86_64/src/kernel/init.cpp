#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__x86_64__)
#error "AlkOS needs to be compiled with a x86_64-elf compiler"
#endif

#include <cpuid.h>
#include <expected.hpp>
#include <hal/debug.hpp>
#include <internal/formats.hpp>

#include "trace_framework.hpp"

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
void EnableNXE();

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

TODO_WHEN_MULTICORE
// TODO: replace when core
extern "C" void cdecl_SetKernelGs(Sched::Thread *thread)
{
    cpu::SetMSR(arch::kIa32GsKernelBase, thread->gs_base);
}

namespace arch
{
void ArchInit(const RawBootArguments &)
{
    hal::SetCoreLocalData(nullptr);

    DEBUG_INFO_BOOT("In ArchInit...");
    DEBUG_INFO_BOOT("CPU Model: %d / %08X", GetCpuModel(), GetCpuModel());

    DEBUG_INFO_BOOT("Setting up CPU features...");
    BlockHardwareInterrupts();

    /* NOTE: sequence is important */
    EnableOSXSave();
    EnableSSE();
    EnableAVX();
    EnableNXE();

    HardwareModule::Init();
    HardwareModule::Get().GetInterrupts().FirstStageInit();

    /* Zero mem core local as no global constructors available yet */
    memset(&g_CoreLocal, 0, sizeof(hardware::CoreLocal));
    cpu::SetMSR(kIa32GsBase, reinterpret_cast<u64>(&g_CoreLocal));
    cpu::SetMSR(kIa32GsKernelBase, reinterpret_cast<u64>(&g_CoreLocal));
    InitializeCoreLocal();

    trace::AdvanceTracingStage();

    EnableHardwareInterrupts();

    DEBUG_INFO_BOOT("Leaving ArchInit");
}
}  // namespace arch
