// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_SCHEDULING_POLICIES_MQAPS_POLICY_HPP_
#define KERNEL_SRC_SCHEDULING_POLICIES_MQAPS_POLICY_HPP_

#include "data_structures/priority_queues/bitmap_pq.hpp"
#include "scheduling/policy.hpp"
#include "scheduling/thread.hpp"
#include "trace_framework.hpp"

namespace Sched
{

/**
 * @brief Multi-Queue Adaptive Priority Scheduling (MQAPS)
 *
 * Implements a dual-queue strategy:
 * 1. Ready Queue (RQ): For high-priority tasks (Priority > 2).
 * 2. Secondary Queue (SQ): For lower-priority tasks (Priority <= 2).
 *
 * Features:
 * - Dynamic Time Quantum (TQ):
 *   - If queue has multiple processes: TQ = Average Burst Time of queue.
 *   - If queue empty (single process context): TQ = Process Burst Time (Current Average).
 * - Priority Adaptation:
 *   - If task finishes in TQ: Priority remains (or process exits).
 *   - If task incomplete: Priority decrements.
 *   - Tasks drop from RQ to SQ if priority falls to threshold (2).
 */
class MQAPSPolicy : public PolicyImpl
{
    public:
    static constexpr u8 kMaxPriority = 5;
    static constexpr u8 kThreshold   = 2;
    static constexpr u64 kDefaultTQ  = 10'000'000;  // 10ms

    MQAPSPolicy()  = default;
    ~MQAPSPolicy() = default;

    NODISCARD Thread *PickNextTask()
    {
        TRACE_INFO_SCHEDULING("MQAPS: PickNextTask called");

        // 1. Try Ready Queue (RQ) - High Priority
        if (!rq_.IsEmpty()) {
            Thread *t = rq_.DeleteMax();
            UpdateStats(t, true, false);  // Remove from RQ stats
            TRACE_INFO_SCHEDULING(
                "MQAPS: Picked thread TID=%llu from RQ (priority=%u)", t->tid, t->flags.priority
            );
            return t;
        }

        // 2. Try Secondary Queue (SQ) - Low Priority
        if (!sq_.IsEmpty()) {
            Thread *t = sq_.DeleteMax();
            UpdateStats(t, false, false);  // Remove from SQ stats
            TRACE_INFO_SCHEDULING(
                "MQAPS: Picked thread TID=%llu from SQ (priority=%u)", t->tid, t->flags.priority
            );
            return t;
        }

        TRACE_INFO_SCHEDULING("MQAPS: No threads available in RQ or SQ");
        return nullptr;
    }

    void AddTask(Thread *thread)
    {
        ASSERT_NOT_NULL(thread);
        ASSERT_EQ(thread->state, ThreadState::kReady);
        ASSERT_LT(thread->flags.priority, kMaxPriority);

        // Logic: if priority > 2, goes to RQ. Else SQ.
        bool to_rq = thread->flags.priority > kThreshold;

        if (to_rq) {
            rq_.Insert(thread, thread->flags.priority);
            UpdateStats(thread, true, true);
            TRACE_INFO_SCHEDULING(
                "MQAPS: Added TID=%llu to RQ (priority=%u, avg_burst=%llu ns)", thread->tid,
                thread->flags.priority, thread->avg_burst_ns
            );
        } else {
            sq_.Insert(thread, thread->flags.priority);
            UpdateStats(thread, false, true);
            TRACE_INFO_SCHEDULING(
                "MQAPS: Added TID=%llu to SQ (priority=%u, avg_burst=%llu ns)", thread->tid,
                thread->flags.priority, thread->avg_burst_ns
            );
        }
    }

    NODISCARD u64 GetPreemptTime(Thread *thread)
    {
        ASSERT_NOT_NULL(thread);

        u64 tq = CalculateTQ(thread);

        if (thread->state != ThreadState::kRunning) {
            TRACE_INFO_SCHEDULING(
                "MQAPS: GetPreemptTime TID=%llu (not running) -> TQ=%llu ns", thread->tid, tq
            );
            return tq;
        }

        // Thread is running, check if quantum expired
        const u64 cpu_time = thread->CalculateCpuTime();

        if (cpu_time >= tq) {
            TRACE_INFO_SCHEDULING(
                "MQAPS: TID=%llu quantum expired (cpu_time=%llu >= tq=%llu)", thread->tid, cpu_time,
                tq
            );
            OnThreadQuantumExpired(thread);
            return 0;  // Preempt
        }

        const u64 remaining = tq - cpu_time;
        TRACE_INFO_SCHEDULING(
            "MQAPS: GetPreemptTime TID=%llu -> remaining=%llu ns (tq=%llu, cpu_time=%llu)",
            thread->tid, remaining, tq, cpu_time
        );
        return remaining;
    }

    NODISCARD bool IsFirstHigherPriority(Thread *first, Thread *second)
    {
        ASSERT_NOT_NULL(first);
        ASSERT_NOT_NULL(second);

        return first->flags.priority > second->flags.priority;
    }

    NODISCARD bool ValidateThreadFlags(const ThreadFlags *flags)
    {
        return flags->priority >= kMaxPriority;
    }

    void OnThreadYield(Thread *thread)
    {
        // Update burst statistics
        u64 burst = thread->CalculateCpuTime();
        TRACE_INFO_SCHEDULING(
            "MQAPS: OnThreadYield TID=%llu (burst=%llu ns, old_avg_burst=%llu ns)", thread->tid,
            burst, thread->avg_burst_ns
        );
        UpdateThreadBurst(thread, burst);
    }

    private:
    // ------------------------------
    // Class fields
    // ------------------------------

    data_structures::BitmapPriorityQueue<Thread, kMaxPriority, kSchedulingIntrusiveLevel>
        rq_;  // Priority > kThreshold
    data_structures::BitmapPriorityQueue<Thread, kMaxPriority, kSchedulingIntrusiveLevel>
        sq_;  // Priority <= kThreshold

    // Stats for dynamic TQ calculation
    u64 rq_burst_sum_ = 0;
    size_t rq_count_  = 0;

    u64 sq_burst_sum_ = 0;
    size_t sq_count_  = 0;

    // ------------------------------
    // Helpers
    // ------------------------------

    void OnThreadQuantumExpired(Thread *thread)
    {
        // 1. Decrement Priority
        const u8 old_priority = thread->flags.priority;
        if (thread->flags.priority > 1) {
            thread->flags.priority--;
        }

        // 2. Update Burst Stats
        u64 burst = thread->CalculateCpuTime();
        TRACE_INFO_SCHEDULING(
            "MQAPS: QuantumExpired TID=%llu (priority: %u->%u, burst=%llu ns)", thread->tid,
            old_priority, thread->flags.priority, burst
        );
        UpdateThreadBurst(thread, burst);
    }

    void UpdateThreadBurst(Thread *thread, u64 burst)
    {
        const u64 old_avg     = thread->avg_burst_ns;
        thread->last_burst_ns = burst;
        // Simple moving average
        if (thread->avg_burst_ns == 0) {
            thread->avg_burst_ns = burst;
        } else {
            thread->avg_burst_ns = (thread->avg_burst_ns + burst) / 2;
        }

        // Safety clamp for TQ
        if (thread->avg_burst_ns < 1'000'000)
            thread->avg_burst_ns = 1'000'000;  // Min 1ms

        TRACE_INFO_SCHEDULING(
            "MQAPS: UpdateBurst TID=%llu (burst=%llu ns, avg_burst: %llu->%llu ns)", thread->tid,
            burst, old_avg, thread->avg_burst_ns
        );
    }

    void UpdateStats(Thread *t, bool is_rq, bool add)
    {
        u64 burst = t->avg_burst_ns == 0 ? kDefaultTQ : t->avg_burst_ns;
        if (is_rq) {
            if (add) {
                rq_burst_sum_ += burst;
                rq_count_++;
            } else {
                if (rq_count_ > 0) {
                    rq_burst_sum_ -= burst;
                    rq_count_--;
                }
            }
        } else {
            if (add) {
                sq_burst_sum_ += burst;
                sq_count_++;
            } else {
                if (sq_count_ > 0) {
                    sq_burst_sum_ -= burst;
                    sq_count_--;
                }
            }
        }
    }

    u64 CalculateTQ(Thread *t)
    {
        bool is_rq = t->flags.priority > kThreshold;

        // "If queue empty (count <= 1 running task implied), TQ = Process Burst"
        // "If queue not empty, TQ = Avg of Queue"

        // Note: When GetPreemptTime is called for a running thread, it is NOT in the queue
        // structure. So count represents other waiting threads.

        u64 tq;
        if (is_rq) {
            if (rq_count_ == 0) {
                tq = t->avg_burst_ns == 0 ? kDefaultTQ : t->avg_burst_ns;
                TRACE_INFO_SCHEDULING(
                    "MQAPS: CalculateTQ TID=%llu (RQ empty) -> using thread avg_burst=%llu ns",
                    t->tid, tq
                );
            } else {
                tq = rq_burst_sum_ / rq_count_;
                TRACE_INFO_SCHEDULING(
                    "MQAPS: CalculateTQ TID=%llu (RQ count=%zu, sum=%llu) -> TQ=%llu ns", t->tid,
                    rq_count_, rq_burst_sum_, tq
                );
            }
        } else {
            if (sq_count_ == 0) {
                tq = t->avg_burst_ns == 0 ? kDefaultTQ : t->avg_burst_ns;
                TRACE_INFO_SCHEDULING(
                    "MQAPS: CalculateTQ TID=%llu (SQ empty) -> using thread avg_burst=%llu ns",
                    t->tid, tq
                );
            } else {
                tq = sq_burst_sum_ / sq_count_;
                TRACE_INFO_SCHEDULING(
                    "MQAPS: CalculateTQ TID=%llu (SQ count=%zu, sum=%llu) -> TQ=%llu ns", t->tid,
                    sq_count_, sq_burst_sum_, tq
                );
            }
        }
        return tq;
    }
};

}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_POLICIES_MQAPS_POLICY_HPP_
