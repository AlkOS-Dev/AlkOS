#include "modules/window.hpp"

#include <string.h>
#include <cstddef>
#include <template/scope_guard.hpp>

#include "trace_framework.hpp"

namespace internal
{

using namespace Mem;

WindowModule::WindowModule() noexcept
{
    DEBUG_INFO_GENERAL("WindowModule Initialized. Active Session: 0 (Kernel)");
}

std::expected<void *, Mem::MemError> WindowModule::CreateSession()
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

    page_guard.dismiss();
    return virt;
}

void WindowModule::SwitchSession(size_t index)
{
    // Validate index (0 is kernel, 1..Size are user sessions)
    if (index >= sessions_.Size()) {
        return;
    }

    if (active_session_idx_ == index) {
        return;
    }

    DEBUG_INFO_GENERAL("Switching to Session %zu", index);
    active_session_idx_ = index;
    RefreshScreen();
}

void WindowModule::RefreshScreen()
{
    if (active_session_idx_ == 0) {
        // Kernel session is always drawing to the front buffer directly.
        // We don't have a backbuffer for it to restore from currently.
        return;
    }

    // Restore User App
    GraphicSession &session = sessions_[active_session_idx_];
    BlitSession(session);
}

void WindowModule::Blit(Sched::Pid pid)
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

std::expected<BufferInfo, Mem::MemError> WindowModule::AllocUserBuffer()
{
    auto &pmm = ::MemoryModule::Get().GetBuddyPmm();

    size_t buffer_size = video_module_.GetFramebuffer().CalculateSize();

    u8 order      = Mem::BuddyPmm::SizeToPageOrder(buffer_size);
    auto phys_res = pmm.Alloc({.order = order});
    RET_UNEXPECTED_IF_ERR(phys_res);

    return BufferInfo{.phys_buffer = *phys_res, .size_bytes = buffer_size};
}

size_t WindowModule::RegisterGraphicsSession(Sched::Pid pid, BufferInfo buffer)
{
    GraphicSession session;
    session.owner_pid   = pid;
    session.phys_buffer = buffer.phys_buffer;
    session.size_bytes  = buffer.size_bytes;
    session.is_active   = false;

    sessions_.Push(session);
    size_t session_id = sessions_.Size() - 1;

    DEBUG_INFO_GENERAL("Created Graphic Session %zu for PID %llu", session_id, pid);
    return session_id;
}

void WindowModule::BlitSession(const GraphicSession &session)
{
    auto &screen = video_module_.GetFramebuffer().GetSurface();

    VPtr<void> vram_dst             = screen.GetRawBuffer();
    const VPtr<void> backbuffer_src = Mem::PhysToVirt(session.phys_buffer);
    memcpy(vram_dst, backbuffer_src, session.size_bytes);
}

std::tuple<GraphicSession *, size_t> WindowModule::FindSession(Sched::Pid pid)
{
    for (size_t i = 0; i < sessions_.Size(); ++i) {
        if (sessions_[i].owner_pid == pid) {
            return {&sessions_[i], i};
        }
    }
    return {nullptr, 0};
}

}  // namespace internal
