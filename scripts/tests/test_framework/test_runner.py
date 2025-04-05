import logging
import select
import subprocess
import time

from .test_commands import TEST_COMMAND_SPECIFIER, TEST_SUCCESS_COMMAND, TEST_FAIL_COMMAND, TEST_DISPLAY_STOP_COMMAND_IN
from .test_data import MAX_ALKOS_TEST_TIME, MAX_ALKOS_WAIT_SYNC_TIME, TestState
from .test_data import TestInfo
from .test_log import TestLog, TestFile
from .test_utils import print_red, print_green


def _process_test_state(test_state: TestState, line: str) -> TestState:
    assert TEST_COMMAND_SPECIFIER in line

    if test_state == TestState.UNKNOWN:
        if TEST_SUCCESS_COMMAND in line:
            return TestState.SUCCESS
        elif TEST_FAIL_COMMAND in line:
            return TestState.FAILURE

    elif test_state == TestState.SUCCESS:
        if TEST_SUCCESS_COMMAND in line:
            return TestState.ERROR_DOUBLE_SUCCESS
        if TEST_FAIL_COMMAND in line:
            return TestState.ERROR_FAIL_AFTER_SUCCESS

    elif test_state == TestState.FAILURE:
        if TEST_FAIL_COMMAND in line:
            return TestState.ERROR_FAIL_AFTER_FAIL
        if TEST_SUCCESS_COMMAND in line:
            return TestState.ERROR_SUCCESS_AFTER_FAIL

    return test_state


def _run_test(path: str, info: TestInfo, log_file: TestFile) -> bool:
    state = TestState.UNKNOWN
    should_process = True

    try:
        alkos = subprocess.Popen(
            [path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        logging.info("AlkOS started for test run...")
        start_time = time.perf_counter_ns()

        # Read lines
        while should_process:
            reads = [alkos.stdout, alkos.stderr]

            curr_time = time.perf_counter_ns()
            readable, _, _ = select.select(reads, [], [], MAX_ALKOS_TEST_TIME - (curr_time - start_time) * 1e-9)

            if not readable:
                logging.error("Select returned not readable...")

                alkos.kill()
                alkos.wait()

                logging.info("AlkOS killed and waited...")
                stdout, stderr = alkos.communicate()

                if stdout:
                    log_file.write(stdout)

                if stderr:
                    log_file.write(stderr)

                raise Exception("Readable failed -> timed out or stream failed")

            for stream in readable:
                line = stream.readline()

                if not line:
                    logging.warn("Stream returned None on parsing, gentle abort...")
                    should_process = False
                    break

                if TEST_COMMAND_SPECIFIER in line:
                    state = _process_test_state(state, line)

                # After "LISTEND" send the test name
                if TEST_DISPLAY_STOP_COMMAND_IN in line:
                    state = _process_test_state(state, line)

                    logging.info("Test name send...")
                    alkos.stdin.write(info.test_name + '\r')
                    alkos.stdin.flush()

                log_file.write(line)

            if alkos.poll() is not None:
                logging.info("AlkOS dead -> detected on poll -> stopping...")
                break

    except Exception as e:
        logging.error(f"Failed to run the test ({info.test_name}): {e}")

        print(f"[ERROR] Unexpected error running alkos: {e}")
        exit(1)

    try:
        alkos.wait(MAX_ALKOS_WAIT_SYNC_TIME)
        stdout, stderr = alkos.communicate()
        logging.info("AlkOS killed and waited on cleanup...")

        if stdout:
            log_file.write(stdout)

        if stderr:
            log_file.write(stderr)

    except Exception as e:
        logging.error(f"Unexpected error occurred when waiting for AlkOS: {e}...")

        print(f"[ERROR] Unexpected error waiting for alkos: {e}")
        exit(1)

    if state != TestState.SUCCESS and state != TestState.FAILURE:
        print(f"[WARNING] Test failed with unexpected error: {str(state)}")

    return state == TestState.SUCCESS


def run_test(path: str, info: TestInfo, logger: TestLog) -> bool:
    with logger.save_log_with_context(info) as log_file:
        result = _run_test(path, info, log_file)

        if result:
            print_green(f"[PASS] Test: {info.test_name} passed...")
        else:
            print_red(f"[FAIL] Test: {info.test_name} failed...")

        return result
