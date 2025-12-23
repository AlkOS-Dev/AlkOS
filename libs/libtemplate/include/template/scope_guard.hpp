#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_SCOPE_GUARD_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_SCOPE_GUARD_HPP_

#include <assert.h>
#include <utility.hpp>

namespace template_lib
{

template <class F>
class ScopeGuard
{
    public:
    // ------------------------------
    // Class construction
    // ------------------------------

    explicit ScopeGuard(F &&f) : f_(std::forward<F>(f)) {}

    ScopeGuard(ScopeGuard &&other) noexcept : f_(std::move(other.f_)), dismissed_(other.dismissed_)
    {
        other.dismissed_ = true;
    }

    ScopeGuard(ScopeGuard &)            = delete;
    ScopeGuard &operator=(ScopeGuard &) = delete;

    ~ScopeGuard()
    {
        if (!dismissed_) {
            f_();
        }
    }

    // ------------------------------
    // Class interaction
    // ------------------------------

    void dismiss() { dismissed_ = true; }

    // ------------------------------
    // Class Fields
    // ------------------------------

    private:
    F f_{};
    bool dismissed_{};
};

template <class F>
class BatchedScopeGuard
{
    public:
    // ------------------------------
    // Class construction
    // ------------------------------

    explicit BatchedScopeGuard(bool &dismissed, F &&f)
        : f_(std::forward<F>(f)), dismissed_(dismissed)
    {
    }

    BatchedScopeGuard(BatchedScopeGuard &&)            = delete;
    BatchedScopeGuard &operator=(BatchedScopeGuard &&) = delete;

    BatchedScopeGuard(BatchedScopeGuard &)            = delete;
    BatchedScopeGuard &operator=(BatchedScopeGuard &) = delete;

    ~BatchedScopeGuard()
    {
        if (!dismissed_) {
            f_();
        }
    }

    // ------------------------------
    // Class Fields
    // ------------------------------

    private:
    F f_;
    bool &dismissed_;
};

}  // namespace template_lib

#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_SCOPE_GUARD_HPP_
