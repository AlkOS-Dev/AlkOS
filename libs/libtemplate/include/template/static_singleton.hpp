#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_STATIC_SINGLETON_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_STATIC_SINGLETON_HPP_

#include <assert.h>
#include <new.hpp>
#include <type_traits.hpp>
#include <utility.hpp>

#include "special_members.hpp"

namespace template_lib
{
// ------------------------------
// Static singleton
// ------------------------------

class StaticSingletonHelper : public NoCopy
{
    protected:
    /* Non instantiable */
    StaticSingletonHelper()  = default;
    ~StaticSingletonHelper() = default;
};

template <class T>
concept DerivedFromHelper = std::is_base_of_v<StaticSingletonHelper, T>;

template <class T>
    requires DerivedFromHelper<T>
class SingletonInstanceCreator
{
    struct InstanceHelper final : T {
        /* Makes protected constructor accessible */

        template <class... Args>
        explicit InstanceHelper(Args &&...args) noexcept : T(std::forward<Args>(args)...)
        {
        }
    };

    public:
    // ------------------------------------
    // Static Accessors and Utilities
    // ------------------------------------

    FORCE_INLINE_F T &Get() noexcept
    {
        assert(is_instance_inited_ && "Not inited Singleton instance!");
        return *reinterpret_cast<T *>(instance_memory_);
    }

    void Destroy() noexcept
    {
        Get().~T();
        is_instance_inited_ = false;
    }

    FORCE_INLINE_F bool IsInited() const noexcept { return is_instance_inited_; }

    template <class... Args>
    T &Init(Args &&...args) noexcept
    {
        assert(!IsInited() && "Singleton instance already inited!");
        [[maybe_unused]] auto ptr =
            new (instance_memory_) InstanceHelper(std::forward<Args>(args)...);
        assert(ptr == reinterpret_cast<T *>(instance_memory_));

        is_instance_inited_ = true;
        return Get();
    }

    protected:
    // ------------------------------
    // Static memory
    // ------------------------------

    bool is_instance_inited_ = false;
    alignas(alignof(T)) unsigned char instance_memory_[sizeof(T)]{};
};

template <class T>
    requires DerivedFromHelper<T>
class StaticSingleton
{
    public:
    // ------------------------------------
    // Static Accessors and Utilities
    // ------------------------------------

    FORCE_INLINE_F static T &Get() noexcept { return instance_creator_.Get(); }

    static void Destroy() noexcept { instance_creator_.Destroy(); }

    FORCE_INLINE_F static bool IsInited() noexcept { return instance_creator_.IsInited(); }

    template <class... Args>
    static T &Init(Args &&...args) noexcept
    {
        return instance_creator_.Init(std::forward<Args>(args)...);
    }

    protected:
    // ------------------------------
    // Static memory
    // ------------------------------

    static SingletonInstanceCreator<T> instance_creator_;
};

template <class T>
    requires DerivedFromHelper<T>
SingletonInstanceCreator<T> StaticSingleton<T>::instance_creator_;
}  // namespace template_lib
#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_STATIC_SINGLETON_HPP_
