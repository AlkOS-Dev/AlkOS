#!/bin/python

import test_framework.main as tf
import os

# This Python script is designed to run tests within a continuous integration/continuous deployment (CICD) process.
# It defines multiple test "runs" (TestRuns) that represent groups of related tests, where all tests in the group must
# succeed together. The tests can be filtered or blocked based on certain conditions. The script helps manage test
# executions, specify filters for tests to run, and block specific tests that are known to have issues.

# A TestRun represents a single logical group of tests. Each TestRun contains:
# - 'name': A human-readable name for the test group.
# - 'test_filters': A list of filters that can be applied to select a subset of tests to run (e.g., by test name).
# - 'test_blockers': A list of tests that are known to fail or should be skipped due to issues (e.g., bugs).
#
# **Important Note on Blocked Tests**:
# Each entry in the 'test_blockers' list should include a comment explaining why the test is disabled.
# These tests are typically blocked because they have known issues or bugs, and as such, should not be run
# during this test run. The comment should indicate that the test is considered by default to be fixed in the future.
# Additionally, an issue should be created for each blocked test to track the problem and its resolution.
# This ensures that any known issues are documented and can be tracked until they are resolved, preventing wasted
# resources running tests that are expected to fail.

# The TEST_RUNS list holds all the test groups you want to execute. Each entry in this list is an instance of the
# TestRun class, and you can configure it with different filters and blockers as needed.

class TestRun:
    name: str
    test_filters: list[str]
    test_blockers: list[str]

    def __init__(self, name: str, test_filters: list[str], test_blockers: list[str]) -> None:
        self.name = name
        self.test_filters = test_filters
        self.test_blockers = test_blockers


TEST_RUNS: list[TestRun] = [
    TestRun(
        "Full test run",
        [],
        [
            "SnprintfTest_BufferSizeHandling",  # Fails <- issue added on low prio
            "SnprintfTest_EdgeCases",  # Fails <- issue added on low prio
        ]
    )
]


def prepare_command(test_run: TestRun) -> list[str]:
    args = []

    args.append("--path")
    script_dir = os.path.dirname(os.path.realpath(__file__))
    args.append(os.path.join(script_dir, "./run_test_alkos.bash"))

    if len(test_run.test_filters) > 0:
        args.append("--filter")

    for test_filter in test_run.test_filters:
        args.append(test_filter)

    args.append("--block")
    for test_blocker in test_run.test_blockers:
        args.append(test_blocker)

    return args


def run_command(args: list[str]) -> None:
    tf.main(args)


def main() -> None:
    for test_run in TEST_RUNS:
        print(f"[TEST] Running test run: {test_run.name}")

        command = prepare_command(test_run)
        run_command(command)


if __name__ == "__main__":
    main()
