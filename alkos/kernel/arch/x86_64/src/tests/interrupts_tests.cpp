/* internal includes */
#include <assert.h>
#include <arch_utils.hpp>
#include <extensions/bit.hpp>

// ------------------------------
// Preserve cpu state test
// ------------------------------

// TODO - Use test_module/test.hpp
extern "C" void PreserveCpuStateTest()
{
    /* current interrupt idx picked for testing */
    static constexpr u8 kTestInterrupt = 48;

    BlockHardwareInterrupts();

    const auto initial_cpu_state = DumpCpuState();
    initial_cpu_state.DumpStateDesc();

    InvokeInterrupt(kTestInterrupt);

    const auto final_cpu_state = DumpCpuState();
    final_cpu_state.DumpStateDesc();

    for (const u64 &cpu_state : final_cpu_state.general_purpose_registers) {
        R_ASSERT_NEQ(kBitMask64, cpu_state);
    }
}
