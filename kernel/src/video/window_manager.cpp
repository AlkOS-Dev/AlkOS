#include "video/window_manager.hpp"

#include <string.h>
#include <template/scope_guard.hpp>

#include "hardware/core_local.hpp"
#include "modules/memory.hpp"
#include "modules/scheduling.hpp"
#include "trace_framework.hpp"

namespace Video
{

using namespace Mem;

void WindowManager::Init(Framebuffer &fb)
{
    framebuffer_ = &fb;
    DEBUG_INFO_GENERAL("WindowManager Initialized.");
}

std::expected<void *, Mem::MemError> WindowManager::CreateSession()
{
    auto &vmm = ::MemoryModule::Get().GetVmm();
    auto &pmm = ::MemoryModule::Get().GetBuddyPmm();
    auto pid  = hardware::GetRunningPid();

    // Alloc the user buffer
    auto buffer_res = AllocUserBuffer();
    RET_UNEXPECTED_IF_ERR(buffer_res);
    BufferInfo buffer = *buffer_res;

    template_lib::ScopeGuard page_guard([&]() {
        pmm.Free(buffer.phys_buffer);
    });

    memset(Mem::PhysToVirt(buffer.phys_buffer), 0, buffer.size_bytes);

    // Map into User Space of the calling process
    auto proc_res = ::SchedulingModule::Get().GetProcesses().GetProcess(pid);
    RET_UNEXPECTED_IF(!proc_res, MemError::NotFound);
    auto *proc = *proc_res;

    auto virt_res =
        vmm.MapUserBackbuffer(proc->address_space, buffer.phys_buffer, buffer.size_bytes);
    RET_UNEXPECTED_IF_ERR(virt_res);
    VPtr<void> virt = *virt_res;

    // Store Session Metadata
    GraphicSessionNode *node = RegisterGraphicsSession(pid, buffer);

    // Switch focus to new app immediately
    SwitchSession(node);
    DEBUG_INFO_GENERAL(
        "CreateSession: Switched to session (PID %llu). Active node is now %p", pid, active_session_
    );

    page_guard.Dismiss();
    return virt;
}

void WindowManager::SwitchSession(GraphicSessionNode *node)
{
    if (!node) {
        return;
    }

    if (active_session_ == node) {
        return;
    }

    DEBUG_INFO_GENERAL(
        "Switching Session: Old %p -> New %p (PID %llu)", active_session_, node,
        node->data.owner_pid
    );
    active_session_ = node;
    RefreshScreen();
}

void WindowManager::SwitchToNextSession()
{
    if (sessions_.Empty()) {
        return;
    }

    // If no active session, switch to the first one
    if (!active_session_) {
        SwitchSession(sessions_.GetHead());
        return;
    }

    // Use active_session_ node pointer directly for fast navigation
    if (active_session_->next) {
        SwitchSession(active_session_->next);
    } else {
        // Wrap around to the first session
        SwitchSession(sessions_.GetHead());
    }
}

void WindowManager::ReleaseSession(Sched::Pid pid)
{
    for (auto it = sessions_.begin(); it != sessions_.end(); ++it) {
        auto &session = *it;
        if (session.owner_pid == pid) {
            DEBUG_INFO_GENERAL("Releasing Session owned by PID %llu", pid);

            auto *node = it.GetNode();

            // If active, switch to next session
            if (active_session_ == node) {
                SwitchToNextSession();
                // If we switched to ourselves (only session), clear active
                if (active_session_ == node) {
                    active_session_ = nullptr;
                }
            }

            // Free backing store
            auto &pmm = ::MemoryModule::Get().GetBuddyPmm();
            pmm.Free(session.buffer_info.phys_buffer);

            // Remove session from list
            sessions_.Remove(node);
            return;
        }
    }
}

void WindowManager::SetFocus(Sched::Pid pid)
{
    auto caller = hardware::GetRunningPid();
    for (auto &session : sessions_) {
        if (session.owner_pid == caller || session.focused_pid == caller) {
            session.focused_pid = pid;
            DEBUG_INFO_GENERAL("WindowManager: Transferred focus to PID %llu", pid);
            return;
        }
    }
}

Sched::Pid WindowManager::GetActiveSessionFocusedPid()
{
    if (!active_session_) {
        return {0, 0};
    }
    return active_session_->data.focused_pid;
}

void WindowManager::ReleaseFocus(Sched::Pid pid)
{
    for (auto &session : sessions_) {
        if (session.focused_pid == pid) {
            session.focused_pid = session.owner_pid;
            DEBUG_INFO_GENERAL(
                "WindowManager: Returned focus to owner PID %llu", session.owner_pid
            );
            return;
        }
    }
}

void WindowManager::RefreshScreen()
{
    if (!active_session_) {
        return;
    }

    // Restore User App
    BlitSession(active_session_->data);
}

void WindowManager::Blit(Sched::Pid pid)
{
    // Find if this PID owns a session
    auto *node = FindSession(pid);

    if (!node) {
        return;
    }

    if (active_session_ != node) {
        // If not active, the data is safely sitting in the session.phys_buffer (RAM),
        // ready to be restored when the user switches back.
        return;
    }

    BlitSession(node->data);
}

std::expected<BufferInfo, Mem::MemError> WindowManager::AllocUserBuffer()
{
    auto &pmm = ::MemoryModule::Get().GetBuddyPmm();

    ASSERT_NOT_NULL(framebuffer_);
    size_t buffer_size = framebuffer_->CalculateSize();

    u8 order      = Mem::BuddyPmm::SizeToPageOrder(buffer_size);
    auto phys_res = pmm.Alloc({.order = order});
    RET_UNEXPECTED_IF_ERR(phys_res);

    return BufferInfo{.phys_buffer = *phys_res, .size_bytes = buffer_size};
}

GraphicSessionNode *WindowManager::RegisterGraphicsSession(Sched::Pid pid, BufferInfo buffer)
{
    GraphicSession session;
    session.owner_pid   = pid;
    session.focused_pid = pid;
    session.buffer_info = buffer;
    session.is_active   = false;

    auto *node = sessions_.PushBack(session);
    ASSERT_NOT_NULL(node);

    DEBUG_INFO_GENERAL("Created Graphic Session for PID %llu", pid);
    return node;
}

void WindowManager::BlitSession(const GraphicSession &session)
{
    ASSERT_NOT_NULL(framebuffer_);
    auto &screen = framebuffer_->GetSurface();

    VPtr<void> vram_dst             = screen.GetRawBuffer();
    const VPtr<void> backbuffer_src = Mem::PhysToVirt(session.buffer_info.phys_buffer);
    memcpy(vram_dst, backbuffer_src, session.buffer_info.size_bytes);
}

GraphicSessionNode *WindowManager::FindSession(Sched::Pid pid)
{
    for (auto it = sessions_.begin(); it != sessions_.end(); ++it) {
        if (it->owner_pid == pid) {
            return it.GetNode();
        }
    }
    return nullptr;
}

}  // namespace Video
