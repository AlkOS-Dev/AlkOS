// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_SCHEDULING_POLICIES_MLFQ_POLICY_HPP_
#define KERNEL_SRC_SCHEDULING_POLICIES_MLFQ_POLICY_HPP_

#include <assert.h>
#include <data_structures/priority_queues/bitmap_pq.hpp>
#include <defines.hpp>

#include "scheduling/policy.hpp"
#include "scheduling/thread.hpp"

namespace Sched
{

/**
 * Multi-Level Feedback Queue (MLFQ) Scheduling Policy
 *
 * MLFQ is a scheduling algorithm that:
 * - Uses multiple priority queues (levels 0 to kNumLevels-1)
 * - Higher priority queues have shorter time slice
 * - Tasks that use their full time slice are demoted to lower priority
 * - Tasks that yield early (interactive) may be boosted to higher priority
 * - Prevents starvation through periodic priority boosting
 * - Within each queue, schedules in FIFO order
 *
 * Priority Boosting:
 * - All threads are boosted to highest priority level every kBoostIntervalNs to prevent starvation
 */
class MLFQPolicy : public PolicyImpl
{
    public:
    // MLFQ configuration constants
    static constexpr u8 kNumLevels        = 3;
    static constexpr u8 kMaxPriority      = static_cast<u8>(UserPriority::kLast);
    static constexpr u64 kBaseTimeSliceNs = 10'000'000;   // 10ms
    static constexpr u64 kBoostIntervalNs = 100'000'000;  // 100ms

    // ------------------------------
    // Class creation
    // ------------------------------

    MLFQPolicy()  = default;
    ~MLFQPolicy() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD Thread *PickNextTask()
    {
        for (size_t i = 0; i < kNumLevels; ++i) {
            if (!queues_[i].IsEmpty()) {
                return queues_[i].DeleteMin();
            }
        }

        return nullptr;
    }

    void AddTask(Thread *thread)
    {
        ASSERT_NOT_NULL(thread);
        ASSERT_EQ(thread->state, ThreadState::kReady);

        const auto weight = static_cast<u64>(thread->flags.user_priority);
        queues_[thread->flags.priority].Insert(thread, weight);
    }

    NODISCARD u64 GetPreemptTime(Thread *thread)
    {
        ASSERT_NOT_NULL(thread);

        const u64 slice_ns = GetTimeSliceForLevel(thread->flags.priority);
        if (thread->state != ThreadState::kRunning) {
            // Thread is not running, return full slice for its level
            return slice_ns;
        }

        // Thread is running, calculate remaining time in slice
        const u64 cpu_time_ns = thread->CalculateCpuTime();

        if (cpu_time_ns >= slice_ns) {
            OnThreadQuantumExpired(thread);
            return 0;
        }

        return slice_ns - cpu_time_ns;
    }

    NODISCARD bool IsFirstHigherPriority(Thread *first, Thread *second)
    {
        ASSERT_NOT_NULL(first);
        ASSERT_NOT_NULL(second);

        if (first->flags.priority == second->flags.priority) {
            // Same level, use user priority as tiebreaker
            const auto first_weight  = static_cast<u64>(first->flags.user_priority);
            const auto second_weight = static_cast<u64>(second->flags.user_priority);
            return first_weight < second_weight;
        }

        // Lower level number = higher priority
        return first->flags.priority < second->flags.priority;
    }

    NODISCARD bool ValidateThreadFlags(const ThreadFlags *) { return false; }

    void OnThreadYield(Thread *thread)
    {
        ASSERT_NOT_NULL(thread);

        // Boost thread by moving up one level for interactive behavior
        if (thread->flags.priority > 0) {
            --thread->flags.priority;
        }
    }

    void OnThreadQuantumExpired(Thread *thread)
    {
        ASSERT_NOT_NULL(thread);

        // Demote to lower priority if not already at lowest level
        if (thread->flags.priority < kNumLevels - 1) {
            ++thread->flags.priority;
        }
    }

    void OnPeriodicUpdate(u64 current_time_ns)
    {
        ASSERT(current_time_ns >= last_boost_time_ns_);

        if (current_time_ns - last_boost_time_ns_ >= kBoostIntervalNs) {
            last_boost_time_ns_ = current_time_ns;

            // Move all threads from all queues to the highest priority queue
            for (u8 level = 1; level < kNumLevels; ++level) {
                while (!queues_[level].IsEmpty()) {
                    auto *thread           = queues_[level].DeleteMin();
                    thread->flags.priority = 0;
                    queues_[0].Insert(thread, 0);
                }
            }
        }
    }

    void RemoveTask(Thread *thread)
    {
        queues_[thread->flags.priority].Remove(
            thread, static_cast<u64>(thread->flags.user_priority)
        );
    }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    NODISCARD FAST_CALL constexpr u64 GetTimeSliceForLevel(u8 level)
    {
        return kBaseTimeSliceNs * (1ULL << level);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    data_structures::BitmapPriorityQueue<Thread, kMaxPriority, kSchedulingIntrusiveLevel>
        queues_[kNumLevels];
    u64 last_boost_time_ns_{0};
};

}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_POLICIES_MLFQ_POLICY_HPP_
