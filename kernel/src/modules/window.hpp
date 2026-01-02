#ifndef KERNEL_SRC_MODULES_WINDOW_HPP_
#define KERNEL_SRC_MODULES_WINDOW_HPP_

#include <data_structures/array_structures.hpp>
#include <expected.hpp>
#include <template_lib.hpp>
#include <tuple.hpp>

#include "modules/helpers.hpp"
#include "modules/memory.hpp"
#include "modules/scheduling.hpp"
#include "modules/video.hpp"

namespace internal
{

struct BufferInfo {
    Mem::PPtr<Mem::Page> phys_buffer;
    size_t size_bytes;
};

struct GraphicSession {
    Sched::Pid owner_pid;
    bool is_active = false;

    /// The backing store (Physical RAM)
    /// Kernel accesses this via Mem::PhysToVirt to copy to VRAM
    Mem::PPtr<void> phys_buffer;
    size_t size_bytes;
};

class WindowModule : template_lib::StaticSingletonHelper
{
    protected:
    WindowModule() noexcept;

    public:
    /// Called by Syscall: Allocates a buffer, maps it to user, registers session
    std::expected<void *, Mem::MemError> CreateSession();

    /// Called by Syscall: If the caller is the active session, copy buffer to VRAM
    void Blit(Sched::Pid pid);

    /// Switches screen to a specific session
    void SwitchSession(size_t index);

    /// Returns true if the kernel console (Session 0) should be visible
    bool IsKernelSessionActive() const { return active_session_idx_ == 0; }

    private:
    std::expected<BufferInfo, Mem::MemError> AllocUserBuffer();
    size_t RegisterGraphicsSession(Sched::Pid pid, BufferInfo buffer);
    void BlitSession(const GraphicSession &session);
    std::tuple<GraphicSession *, size_t> FindSession(Sched::Pid pid);

    void RefreshScreen();

    static constexpr size_t kMaxSessions = 12;
    data_structures::StaticVector<GraphicSession, kMaxSessions> sessions_;

    size_t active_session_idx_{0};

    internal::VideoModule &video_module_;
};

}  // namespace internal

using WindowModule = template_lib::StaticSingleton<internal::WindowModule>;

#endif  // KERNEL_SRC_MODULES_WINDOW_HPP_
