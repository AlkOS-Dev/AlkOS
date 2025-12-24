#include "core_local.hpp"

Sched::Thread *cdecl_GetCurrentTCB() { return hardware::GetCurrentTCB(); }
void cdecl_SetCurrentTCB(Sched::Thread *tcb) { hardware::SetCurrentTCB(tcb); }
