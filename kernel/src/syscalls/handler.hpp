#ifndef KERNEL_SRC_SYSCALLS_HANDLER_HPP_
#define KERNEL_SRC_SYSCALLS_HANDLER_HPP_

#include <types.h>
#include <type_traits.hpp>
#include <utility.hpp>

#include "hal/types.hpp"

namespace Syscall
{

// Forward declaration
struct SyscallArgs;

// Base handler type
using SyscallHandler =
    hal::reg_t (*)(hal::reg_t, hal::reg_t, hal::reg_t, hal::reg_t, hal::reg_t, hal::reg_t);

// Syscall argument structure
struct SyscallArgs {
    hal::reg_t arg0, arg1, arg2, arg3, arg4, arg5;
};

template <typename T>
concept SyscallRetType = std::is_integral_v<T> || std::is_pointer_v<T> || std::is_same_v<T, void>;

template <typename T>
concept SyscallArgType = std::is_integral_v<T> || std::is_pointer_v<T> || std::is_enum_v<T>;

template <auto TargetFunc>
class SyscallWrapper
{
    public:
    static hal::reg_t Invoke(
        hal::reg_t a0, hal::reg_t a1, hal::reg_t a2, hal::reg_t a3, hal::reg_t a4, hal::reg_t a5
    )
    {
        SyscallArgs args{a0, a1, a2, a3, a4, a5};
        return DeduceAndDispatch(args, TargetFunc);
    }

    private:
    // Argument Extraction and Conversion
    template <size_t Idx>
    static hal::reg_t GetRawArg(const SyscallArgs &args)
    {
        static constexpr hal::reg_t SyscallArgs::*members[] = {
            &SyscallArgs::arg0, &SyscallArgs::arg1, &SyscallArgs::arg2,
            &SyscallArgs::arg3, &SyscallArgs::arg4, &SyscallArgs::arg5
        };
        static_assert(Idx < (sizeof(members) / sizeof(members[0])), "Syscall index out of bounds");
        return args.*(members[Idx]);
    }

    template <typename T>
    static T ConvertArg(hal::reg_t val)
    {
        if constexpr (std::is_pointer_v<T>)
            return reinterpret_cast<T>(val);
        else
            return static_cast<T>(val);
    }

    // Dispatcher
    template <typename Ret, typename Seq, typename... Args>
    struct Dispatch;

    template <typename Ret, size_t... Is, typename... Args>
    struct Dispatch<Ret, std::index_sequence<Is...>, Args...> {
        static hal::reg_t operator()(const SyscallArgs &args)
        {
            if constexpr (std::is_void_v<Ret>) {
                TargetFunc(ConvertArg<Args>(GetRawArg<Is>(args))...);
                return 0;
            } else {
                auto result = TargetFunc(ConvertArg<Args>(GetRawArg<Is>(args))...);

                if constexpr (std::is_pointer_v<Ret>)
                    return reinterpret_cast<hal::reg_t>(result);
                else
                    return static_cast<hal::reg_t>(result);
            }
        }
    };

    // Type Deduction Helper
    template <typename Ret, typename... Args>
        requires SyscallRetType<Ret> && (SyscallArgType<Args> && ...)
    static hal::reg_t DeduceAndDispatch(const SyscallArgs &args, Ret (*)(Args...))
    {
        return Dispatch<Ret, std::index_sequence_for<Args...>, Args...>::operator()(args);
    }
};

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_HANDLER_HPP_
