#ifndef KERNEL_SRC_VIDEO_WINDOW_MANAGER_HPP_
#define KERNEL_SRC_VIDEO_WINDOW_MANAGER_HPP_

#include <types.h>
#include <data_structures/array_structures.hpp>
#include <expected.hpp>
#include <tuple.hpp>

#include "drivers/video/framebuffer.hpp"
#include "mem/error.hpp"
#include "mem/page.hpp"
#include "mem/types.hpp"
#include "scheduling/process.hpp"

namespace Video
{

using Drivers::Video::Framebuffer;

struct BufferInfo {
    Mem::PPtr<Mem::Page> phys_buffer;
    size_t size_bytes;
};

struct GraphicSession {
    Sched::Pid owner_pid;
    Sched::Pid focused_pid;
    bool is_active = false;

    /// The backing store (Physical RAM)
    /// Kernel accesses this via Mem::PhysToVirt to copy to VRAM
    BufferInfo buffer_info;
};

class WindowManager
{
    public:
    WindowManager() = default;

    void Init(Framebuffer &fb);

    /// Called by Syscall: Allocates a buffer, maps it to user, registers session
    std::expected<void *, Mem::MemError> CreateSession();

    /// Called by Syscall: If the caller is the active session, copy buffer to VRAM
    void Blit(Sched::Pid pid);

    /// Switches screen to a specific session
    void SwitchSession(size_t index);
    void SwitchToNextSession();

    void SetFocus(Sched::Pid pid);
    void ReleaseFocus(Sched::Pid pid);
    Sched::Pid GetActiveSessionFocusedPid();

    private:
    std::expected<BufferInfo, Mem::MemError> AllocUserBuffer();
    size_t RegisterGraphicsSession(Sched::Pid pid, BufferInfo buffer);
    void BlitSession(const GraphicSession &session);
    std::tuple<GraphicSession *, size_t> FindSession(Sched::Pid pid);
    void RefreshScreen();

    static constexpr size_t kMaxSessions    = 12;
    static constexpr size_t kInvalidSession = static_cast<size_t>(-1);
    data_structures::StaticVector<GraphicSession, kMaxSessions> sessions_;

    size_t active_session_idx_{kInvalidSession};
    Framebuffer *framebuffer_{nullptr};
};

}  // namespace Video

#endif  // KERNEL_SRC_VIDEO_WINDOW_MANAGER_HPP_
