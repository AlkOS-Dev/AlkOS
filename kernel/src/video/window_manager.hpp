// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_VIDEO_WINDOW_MANAGER_HPP_
#define KERNEL_SRC_VIDEO_WINDOW_MANAGER_HPP_

#include <types.h>
#include <data_structures/linked_list.hpp>
#include <expected.hpp>

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

struct GraphicSession;
using GraphicSessionNode = data_structures::internal::LinkedListNode<GraphicSession>;

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
    void SwitchSession(GraphicSessionNode *node);
    void SwitchToNextSession();
    void ReleaseSession(Sched::Pid pid);

    void SetFocus(Sched::Pid pid);
    void ReleaseFocus(Sched::Pid pid);
    Sched::Pid GetActiveSessionFocusedPid();

    private:
    std::expected<BufferInfo, Mem::MemError> AllocUserBuffer();
    GraphicSessionNode *RegisterGraphicsSession(Sched::Pid pid, BufferInfo buffer);
    void BlitSession(const GraphicSession &session);
    GraphicSessionNode *FindSession(Sched::Pid pid);
    void RefreshScreen();

    static constexpr size_t kMaxSessions = 12;
    data_structures::StaticDoubleLinkedList<GraphicSession, kMaxSessions> sessions_;

    GraphicSessionNode *active_session_{nullptr};
    Framebuffer *framebuffer_{nullptr};
};

}  // namespace Video

#endif  // KERNEL_SRC_VIDEO_WINDOW_MANAGER_HPP_
