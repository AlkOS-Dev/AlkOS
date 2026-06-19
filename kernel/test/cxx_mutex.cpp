// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <mutex.hpp>
#include <sync/spinlock.hpp>
#include <test_module/test.hpp>

class MutexTest : public TestGroupBase
{
    protected:
    Spinlock lock_{};
};

TEST_F(MutexTest, LockGuard)
{
    {
        std::lock_guard lock(lock_);
        R_ASSERT_TRUE(lock_.IsLocked());
    }
    R_ASSERT_FALSE(lock_.IsLocked());
}
