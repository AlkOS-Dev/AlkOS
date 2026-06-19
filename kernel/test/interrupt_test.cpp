// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <test_module/test.hpp>

class InterruptTest : public TestGroupBase
{
};

void ExceptionFailsKernelTest();
FAIL_TEST_F(InterruptTest, ExceptionFailsKernelTest) { ExceptionFailsKernelTest(); }

void ExceptionTestSavesAllRegisters();
TEST_F(InterruptTest, ExceptionTestSavesAllRegisters) { ExceptionTestSavesAllRegisters(); }
