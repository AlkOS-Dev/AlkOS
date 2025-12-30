#include "kworker.hpp"
#include "hal/debug.hpp"
#include "trace_framework.hpp"

void Sched::KWorkerMain()
{
    TRACE_INFO_SCHEDULING("Created new KWorker!");
    while (true) {
        static size_t kSpins = 1'000'000;
        for (size_t i = 0; i < kSpins; i++) {
            hal::Noop();
            hal::Noop();
            hal::Noop();
            hal::Noop();
        }
    }
}
