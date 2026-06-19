// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_SCHEDULING_ERROR_HPP_
#define KERNEL_SRC_SCHEDULING_ERROR_HPP_

#include <assert.h>

namespace Sched
{

enum class Error {
    OutOfMemory,
    ExceededMaxAllowedInstances,
    ProcessNotFound,
    ThreadNotFound,
    ProcessNameTooLong,
    ExecPathNotFound,
    JoiningDetachedThread,
    SelfJoin,
    AlreadyJoined,
    NoPermission,
};

}  // namespace Sched

static constexpr const char *to_string(const Sched::Error &error)
{
    switch (error) {
        case Sched::Error::OutOfMemory:
            return "OutOfMemory";
        case Sched::Error::ExceededMaxAllowedInstances:
            return "ExceededMaxAllowedInstances";
        case Sched::Error::ProcessNotFound:
            return "ProcessNotFound";
        case Sched::Error::ThreadNotFound:
            return "ThreadNotFound";
        case Sched::Error::ProcessNameTooLong:
            return "ProcessNameTooLong";
        case Sched::Error::ExecPathNotFound:
            return "ExecPathNotFound";
        case Sched::Error::JoiningDetachedThread:
            return "JoiningDetachedThread";
        case Sched::Error::SelfJoin:
            return "SelfJoin";
        case Sched::Error::AlreadyJoined:
            return "AlreadyJoined";
        case Sched::Error::NoPermission:
            return "NoPermission";
    }

    return "unknown error";
}

#endif  // KERNEL_SRC_SCHEDULING_ERROR_HPP_
