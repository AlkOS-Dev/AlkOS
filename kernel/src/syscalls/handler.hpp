#ifndef KERNEL_SRC_SYSCALLS_HANDLER_HPP_
#define KERNEL_SRC_SYSCALLS_HANDLER_HPP_

#include <expected.hpp>
#include <span.hpp>
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

namespace internal
{
template <typename T>
struct is_expected : std::false_type {
};

template <typename T, typename E>
struct is_expected<std::expected<T, E>> : std::true_type {
};

template <typename T>
constexpr bool is_expected_v = is_expected<T>::value;

template <typename T>
struct is_span : std::false_type {
};

template <typename T, std::size_t Extent>
struct is_span<std::span<T, Extent>> : std::true_type {
};

template <typename T>
constexpr bool is_span_v = is_span<T>::value;

// Helper to get span element type
template <typename T>
struct span_element_type;

template <typename T, std::size_t Extent>
struct span_element_type<std::span<T, Extent>> {
    using type = T;
};

template <typename T>
using span_element_type_t = span_element_type<T>::type;

template <typename T>
concept ConstructibleFromCString = requires(const char *str) {
    { T(str) };
};

}  // namespace internal

template <typename T>
concept SyscallRetType = std::is_integral_v<T> || std::is_pointer_v<T> || std::is_same_v<T, void> ||
                         internal::is_expected_v<T>;

template <typename T>
concept SyscallArgType =
    std::is_integral_v<T> || std::is_pointer_v<T> || std::is_enum_v<T> || std::is_reference_v<T> ||
    internal::is_span_v<T> || internal::ConstructibleFromCString<std::remove_cvref_t<T>>;

template <auto TargetFunc>
class SyscallWrapper
{
    public:
    FAST_CALL hal::reg_t Invoke(
        hal::reg_t a0, hal::reg_t a1, hal::reg_t a2, hal::reg_t a3, hal::reg_t a4, hal::reg_t a5
    )
    {
        SyscallArgs args{a0, a1, a2, a3, a4, a5};
        return DeduceAndDispatch(args, TargetFunc);
    }

    private:
    // Helper: Calculate how many register slots an argument type consumes
    template <typename T>
    FAST_CALL constexpr size_t ArgRegCount()
    {
        if constexpr (internal::is_span_v<T>) {
            return 2;  // span needs pointer + size
        } else {
            return 1;
        }
    }

    // Helper: Calculate register offset for argument at position Idx
    template <size_t Idx, typename... Args>
    FAST_CALL constexpr size_t CalcRegOffset()
    {
        if constexpr (Idx == 0) {
            return 0;
        } else {
            // Get the tuple of argument types
            using ArgTuple = std::tuple<Args...>;
            using PrevArg  = std::tuple_element_t<Idx - 1, ArgTuple>;
            return CalcRegOffset<Idx - 1, Args...>() + ArgRegCount<PrevArg>();
        }
    }

    // Argument Extraction and Conversion
    template <size_t Idx>
    FAST_CALL hal::reg_t GetRawArg(const SyscallArgs &args)
    {
        static constexpr hal::reg_t SyscallArgs::*members[] = {
            &SyscallArgs::arg0, &SyscallArgs::arg1, &SyscallArgs::arg2,
            &SyscallArgs::arg3, &SyscallArgs::arg4, &SyscallArgs::arg5
        };
        static_assert(
            Idx < (sizeof(members) / sizeof(members[0])), "Too many syscall arguments requested"
        );
        return args.*(members[Idx]);
    }

    template <typename T, size_t RegIdx>
    FAST_CALL T ConvertArg(const SyscallArgs &args)
    {
        if constexpr (internal::is_span_v<T>) {
            // Span needs two consecutive registers: pointer and size
            using ElemType = internal::span_element_type_t<T>;

            auto *ptr = reinterpret_cast<ElemType *>(GetRawArg<RegIdx>(args));
            auto size = GetRawArg<RegIdx + 1>(args);

            return T(ptr, size);
        } else if constexpr (std::is_pointer_v<T>) {
            return reinterpret_cast<T>(GetRawArg<RegIdx>(args));
        } else if constexpr (std::is_reference_v<T>) {
            using BaseType = std::remove_reference_t<T>;

            if constexpr (internal::ConstructibleFromCString<BaseType>) {
                // TODO: Fix for multithreading scenarios
                // Special case: reference to type constructible from const char*
                auto str                 = reinterpret_cast<const char *>(GetRawArg<RegIdx>(args));
                static BaseType temp_obj = BaseType(str);
                return temp_obj;
            } else {
                return *reinterpret_cast<BaseType *>(GetRawArg<RegIdx>(args));
            }
        } else if constexpr (internal::ConstructibleFromCString<T>) {
            // Direct construction from const char* (e.g., vfs::Path from const char*)
            auto str = reinterpret_cast<const char *>(GetRawArg<RegIdx>(args));
            return T(str);
        } else {
            return static_cast<T>(GetRawArg<RegIdx>(args));
        }
    }

    // Dispatcher
    template <typename Ret, typename Seq, typename... Args>
    struct Dispatch;

    template <typename Ret, size_t... Is, typename... Args>
    struct Dispatch<Ret, std::index_sequence<Is...>, Args...> {
        FAST_CALL hal::reg_t operator()(const SyscallArgs &args)
        {
            if constexpr (internal::is_expected_v<Ret>) {
                auto result = TargetFunc(ConvertArg<Args, CalcRegOffset<Is, Args...>()>(args)...);
                if (result.has_value()) {
                    if constexpr (std::is_void_v<typename Ret::value_type>) {
                        return 0;
                    } else {
                        return static_cast<hal::reg_t>(result.value());
                    }
                } else {
                    return static_cast<hal::reg_t>(-static_cast<long>(result.error()));
                }
            } else if constexpr (std::is_void_v<Ret>) {
                TargetFunc(ConvertArg<Args, CalcRegOffset<Is, Args...>()>(args)...);
                return 0;
            } else {
                auto result = TargetFunc(ConvertArg<Args, CalcRegOffset<Is, Args...>()>(args)...);

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
    FAST_CALL hal::reg_t DeduceAndDispatch(const SyscallArgs &args, Ret (*)(Args...))
    {
        return Dispatch<Ret, std::index_sequence_for<Args...>, Args...>::operator()(args);
    }
};

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_HANDLER_HPP_
