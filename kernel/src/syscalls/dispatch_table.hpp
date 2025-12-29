#ifndef KERNEL_SRC_SYSCALLS_DISPATCH_TABLE_HPP_
#define KERNEL_SRC_SYSCALLS_DISPATCH_TABLE_HPP_

#include <sys/calls.h>
#include <defines.hpp>

#include "hal/types.hpp"
#include "syscalls/handler.hpp"

namespace Syscall
{

inline constexpr auto kNotImplemented = static_cast<hal::reg_t>(-1);

struct SyscallDispatchTable {
    consteval SyscallDispatchTable()
    {
        for (size_t i = 0; i < kSysMax; ++i) {
            handlers_[i] = kDefaultHandler;
        }
    }

    // Register a syscall handler at compile time
    template <SyscallNumber num, auto kHandler>
    consteval void RegisterHandler()
    {
        constexpr auto kIdx = static_cast<size_t>(num);
        static_assert(kIdx < kSysMax, "Syscall number out of bounds");

        handlers_[kIdx] = &SyscallWrapper<kHandler>::Invoke;
    }

    private:
    static constexpr auto kDefaultHandler = [](hal::reg_t, hal::reg_t, hal::reg_t, hal::reg_t,
                                               hal::reg_t, hal::reg_t) -> hal::reg_t {
        return kNotImplemented;
    };

    SyscallHandler handlers_[kSysMax];
};

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_DISPATCH_TABLE_HPP_
