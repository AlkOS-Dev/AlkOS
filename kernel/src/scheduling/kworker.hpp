// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_SCHEDULING_KWORKER_HPP_
#define KERNEL_SRC_SCHEDULING_KWORKER_HPP_

namespace Sched
{
struct Pid;

void TraceDumperMain();
void ThreadRipperMain();
void ProcessRipperMain();
void FdHierarchyDumperMain();
void StdoutTracerMain(Pid pid);
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_KWORKER_HPP_
