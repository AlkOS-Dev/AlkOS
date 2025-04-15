#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_

#include <extensions/new.hpp>
#include <extensions/types.hpp>
#include <todo.hpp>

#include "core.hpp"

namespace hardware
{
class CoresController final
{
    public:
    class Core final : public arch::Core
    {
        /* Allow usage of arch constructor */
        using arch::Core::Core;
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    CoresController() = default;

    ~CoresController() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void AllocateCores(size_t num);

    template <class... Args>
    FORCE_INLINE_F void AllocateCore(const size_t idx, Args&&... args)
    {
        byte* ptr = mem_ + idx * sizeof(Core);
        new (reinterpret_cast<Core*>(ptr)) Core(std::forward<Args>(args)...);
    }

    void BootUpAllCores();

    NODISCARD size_t GetNumCores() const noexcept { return num_cores_; }

    NODISCARD Core& GetCore(const size_t idx)
    {
        ASSERT_LT(idx, num_cores_);
        byte* ptr = mem_ + idx * sizeof(Core);

        return *reinterpret_cast<Core*>(ptr);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    TODO_WHEN_VMEM_WORKS
    alignas(Core) byte mem_[sizeof(Core) * 128]{};
    size_t num_cores_{};
};
}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
