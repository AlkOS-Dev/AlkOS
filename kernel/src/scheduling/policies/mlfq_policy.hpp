#ifndef KERNEL_SRC_SCHEDULING_POLICIES_MLFQ_POLICY_HPP_
#define KERNEL_SRC_SCHEDULING_POLICIES_MLFQ_POLICY_HPP_

#include <assert.h>
#include <data_structures/maps/intrusive_rb_tree.hpp>
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
 * - Within each queue, schedules by minimum vruntime for fairness
 *
 * Vruntime scaling:
 * - Used to adjust how quickly a thread's vruntime increases based on its priority level
 * - Higher priority threads have slower vruntime growth, favoring responsiveness
 * - Lower priority threads have faster vruntime growth, ensuring CPU time
 *
 * Priority Boosting:
 * - All threads are boosted to highest priority level every kBoostIntervalNs to prevent starvation
 */
class MLFQPolicy : public PolicyImpl
{
    public:
    static constexpr u8 kNumLevels        = 4;
    static constexpr u64 kBaseTimeSliceNs = 5'000'000;    // 5ms
    static constexpr u64 kBoostIntervalNs = 150'000'000;  // 150ms
    static constexpr u64 kVruntimeScale   = 1024;
    static constexpr std::array<u64, static_cast<size_t>(UserPriority::kLast)> kWeights =
        [] constexpr {
            static constexpr size_t kNumPriorities  = static_cast<size_t>(UserPriority::kLast);
            std::array<u64, kNumPriorities> weights = {};
            weights[0]                              = kVruntimeScale;
            for (u8 level = 1; level < kNumPriorities; ++level) {
                weights[level] = weights[level - 1] * 1.25;
            }
            return weights;
        }();

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

        thread->HookT::key = min_vruntime_;

        thread->flags.priority = 0;
        queues_[0].Insert(thread);
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

        const u8 first_level  = first->flags.priority;
        const u8 second_level = second->flags.priority;

        if (first_level != second_level) {
            return first_level < second_level;
        }

        // Compare by vruntime if same level
        return first->HookT::key < second->HookT::key;
    }

    NODISCARD bool ValidateThreadFlags(const ThreadFlags *) { return false; }

    void OnThreadYield(Thread *thread)
    {
        ASSERT_NOT_NULL(thread);

        if (thread->flags.priority > 0) {
            // Move up one level
            --thread->flags.priority;
        }

        // Update vruntime based on time used (scaled by priority level)
        const u64 time_used_ns = thread->CalculateCpuTime();
        UpdateVruntime(thread, time_used_ns);
    }

    void OnThreadQuantumExpired(Thread *thread)
    {
        const u8 current_level = thread->flags.priority;

        // Demote to lower priority if not already at lowest level
        if (current_level < kNumLevels - 1) {
            ++thread->flags.priority;
        }

        // Update vruntime based on full slice used
        const u64 quantum_ns = GetTimeSliceForLevel(current_level);
        UpdateVruntime(thread, quantum_ns);
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
                    queues_[0].Insert(thread);
                }
            }
        }
    }

    void RemoveTask(Thread *thread) { queues_[thread->flags.priority].Delete(thread); }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    NODISCARD FAST_CALL constexpr u64 GetTimeSliceForLevel(u8 level)
    {
        return kBaseTimeSliceNs * (1ULL << level);
    }

    FORCE_INLINE_F void UpdateVruntime(Thread *thread, u64 delta_ns)
    {
        ASSERT_NOT_NULL(thread);

        thread->HookT::key += delta_ns * kWeights[static_cast<size_t>(UserPriority::kMedium)] /
                              kWeights[static_cast<size_t>(thread->flags.user_priority)];

        if (thread->HookT::key < min_vruntime_ || min_vruntime_ == 0) {
            min_vruntime_ = thread->HookT::key;
        }
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    data_structures::IntrusiveRBTree<Thread, u64, kSchedulingIntrusiveLevel> queues_[kNumLevels];
    using HookT = data_structures::IntrusiveRBTree<Thread, u64, kSchedulingIntrusiveLevel>::HookT;

    u64 last_boost_time_ns_{0};
    u64 min_vruntime_{0};
};

}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_POLICIES_MLFQ_POLICY_HPP_
