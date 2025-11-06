#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_ROLLED_SWITCH_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_ROLLED_SWITCH_HPP_

#include <assert.h>
#include <concepts_ext.hpp>
#include <defines.hpp>
#include <types.hpp>
#include <utility.hpp>

namespace template_lib
{
// ------------------------------
// Rolled Switch
// ------------------------------

template <class ExprT, u64 kMaxValue, u64 kStep, class FuncT, class DefaultFuncT, class... Args>
    requires concepts_ext::Callable<DefaultFuncT, Args...> &&
             concepts_ext::RolledSwitchFunctor<FuncT, ExprT, Args...>
FAST_CALL constexpr void RolledSwitch(
    DefaultFuncT &&default_func, FuncT &&func, const u64 value, Args &&...args
) noexcept
{
    if (value == kMaxValue) {
        func.template operator()<static_cast<ExprT>(kMaxValue)>(std::forward<Args>(args)...);
        return;
    }

    if constexpr (kMaxValue >= kStep) {
        RolledSwitch<ExprT, kMaxValue - kStep, kStep>(
            std::forward<DefaultFuncT>(default_func), std::forward<FuncT>(func), value,
            std::forward<Args>(args)...
        );
        return;
    }

    default_func(std::forward<Args>(args)...);
}

template <class ExprT, u64 kMaxValue, u64 kStep, class FuncT, class DefaultFuncT, class... Args>
    requires concepts_ext::Callable<DefaultFuncT, Args...> &&
             concepts_ext::RolledSwitchFunctor<FuncT, ExprT, Args...>
FAST_CALL constexpr auto RolledSwitchReturnable(
    DefaultFuncT &&default_func, FuncT &&func, const u64 value, Args &&...args
) noexcept
{
    if (value == kMaxValue) {
        return func.template operator()<static_cast<ExprT>(kMaxValue)>(std::forward<Args>(args)...);
    }

    if constexpr (kMaxValue >= kStep) {
        return RolledSwitchReturnable<ExprT, kMaxValue - kStep, kStep>(
            std::forward<DefaultFuncT>(default_func), std::forward<FuncT>(func), value,
            std::forward<Args>(args)...
        );
    }

    return default_func(std::forward<Args>(args)...);
}

template <class ExprT, u64 kMaxValue, u64 kStep, class FuncT, class... Args>
    requires concepts_ext::RolledSwitchFunctor<FuncT, ExprT, Args...>
FAST_CALL constexpr void RolledSwitch(FuncT &&func, const u64 value, Args &&...args) noexcept
{
    RolledSwitch<ExprT, kMaxValue, kStep>(
        []() constexpr FORCE_INLINE_L {
            R_FAIL_ALWAYS("Switch out of range error...");
        },
        std::forward<FuncT>(func), value, std::forward<Args>(args)...
    );
}

template <class ExprT, u64 kMaxValue, u64 kStep, class FuncT, class... Args>
    requires concepts_ext::RolledSwitchFunctor<FuncT, ExprT, Args...>
FAST_CALL constexpr auto RolledSwitchReturnable(
    FuncT &&func, const u64 value, Args &&...args
) noexcept
{
    return RolledSwitchReturnable<ExprT, kMaxValue, kStep>(
        []() constexpr FORCE_INLINE_L {
            R_FAIL_ALWAYS("Switch out of range error...");
        },
        std::forward<FuncT>(func), value, std::forward<Args>(args)...
    );
}
}  // namespace template_lib
#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_ROLLED_SWITCH_HPP_
