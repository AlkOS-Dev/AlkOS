#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_MUTEX_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_MUTEX_HPP_

#include "extensions/concepts_ext.hpp"
#include "extensions/template_lib.hpp"

namespace std
{
// ------------------------------
// std::lock_guard
// ------------------------------

template <concepts_ext::LibCxxCompatibleMutex MutexT>
class lock_guard final : template_lib::NoCopy
{
    // ------------------------------
    // Constructors
    // ------------------------------

    public:
    explicit lock_guard(MutexT &m) : mutex_(m) { m.lock(); }

    // TODO: Add support for std::adopt_lock_t
    // lock_guard( MutexT& m, std::adopt_lock_t t );

    // ------------------------------
    // Destructor
    // ------------------------------

    ~lock_guard() { mutex_.unlock(); }

    // ------------------------------
    // Fields
    // ------------------------------

    private:
    MutexT &mutex_;
};
}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_MUTEX_HPP_
