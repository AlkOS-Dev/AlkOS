// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

/* internal includes */
#include <time.h>
#include <test_module/test.hpp>

class TimeTests : public TestGroupBase
{
    public:
    /* 07.01.2025 1:31pm */
    static constexpr time_t kTestTime1 = 1737117056;
};

TEST_F(TimeTests, TestTime)
{
    tm result;
    localtime_r(&kTestTime1, &result);
}
