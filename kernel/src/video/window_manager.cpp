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
    size_t session_id = RegisterGraphicsSession(pid, buffer);

    // Switch focus to new app immediately
    SwitchSession(session_id);
    DEBUG_INFO_GENERAL(
        "CreateSession: Switched to session %zu. Active is now %zu", session_id, active_session_idx_
    );

    page_guard.Dismiss();
    return virt;
}

void WindowManager::SwitchSession(size_t index)
{
    if (index >= sessions_.Size()) {
        return;
    }

    if (active_session_idx_ == index) {
        return;
    }

    DEBUG_INFO_GENERAL(
        "Switching Session: Old %zu -> New %zu (PID %llu)", active_session_idx_, index,
        sessions_[index].owner_pid
    );
    active_session_idx_ = index;
    RefreshScreen();
}

void WindowManager::SwitchToNextSession()
{
    size_t new_index = (active_session_idx_ + 1) % sessions_.Size();
    SwitchSession(new_index);
}

void WindowManager::SetFocus(Sched::Pid pid)
{
    auto caller = hardware::GetRunningPid();
    for (size_t i = 0; i < sessions_.Size(); ++i) {
        if (sessions_[i].owner_pid == caller || sessions_[i].focused_pid == caller) {
            sessions_[i].focused_pid = pid;
            DEBUG_INFO_GENERAL(
                "WindowManager: Transferred focus in session %zu to PID %llu", i, pid
            );
            return;
        }
    }
}

Sched::Pid WindowManager::GetActiveSessionFocusedPid()
{
    if (active_session_idx_ == kInvalidSession) {
        return {0, 0};
    }
    return sessions_[active_session_idx_].focused_pid;
}

void WindowManager::ReleaseFocus(Sched::Pid pid)
{
    for (size_t i = 0; i < sessions_.Size(); ++i) {
        if (sessions_[i].focused_pid == pid) {
            sessions_[i].focused_pid = sessions_[i].owner_pid;
            DEBUG_INFO_GENERAL(
                "WindowManager: Returned focus in session %zu to owner PID %llu", i,
                sessions_[i].owner_pid
            );
            return;
        }
    }
}

void WindowManager::RefreshScreen()
{
    if (active_session_idx_ == kInvalidSession || active_session_idx_ >= sessions_.Size()) {
        return;
    }

    // Restore User App
    GraphicSession &session = sessions_[active_session_idx_];
    BlitSession(session);
}

void WindowManager::Blit(Sched::Pid pid)
{
    // Find if this PID owns a session
    auto [session, target_idx] = FindSession(pid);

    if (!session) {
        return;
    }

    if (active_session_idx_ != target_idx) {
        // If not active, the data is safely sitting in the session.phys_buffer (RAM),
        // ready to be restored when the user switches back.
        return;
    }

    BlitSession(*session);
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

size_t WindowManager::RegisterGraphicsSession(Sched::Pid pid, BufferInfo buffer)
{
    GraphicSession session;
    session.owner_pid   = pid;
    session.focused_pid = pid;
    session.buffer_info = buffer;
    session.is_active   = false;

    sessions_.Push(session);
    size_t session_id = sessions_.Size() - 1;

    DEBUG_INFO_GENERAL("Created Graphic Session %zu for PID %llu", session_id, pid);
    return session_id;
}

void WindowManager::BlitSession(const GraphicSession &session)
{
    ASSERT_NOT_NULL(framebuffer_);
    auto &screen = framebuffer_->GetSurface();

    VPtr<void> vram_dst             = screen.GetRawBuffer();
    const VPtr<void> backbuffer_src = Mem::PhysToVirt(session.buffer_info.phys_buffer);
    memcpy(vram_dst, backbuffer_src, session.buffer_info.size_bytes);
}

std::tuple<GraphicSession *, size_t> WindowManager::FindSession(Sched::Pid pid)
{
    for (size_t i = 0; i < sessions_.Size(); ++i) {
        if (sessions_[i].owner_pid == pid) {
            return {&sessions_[i], i};
        }
    }
    return {nullptr, kInvalidSession};
}

}  // namespace Video
