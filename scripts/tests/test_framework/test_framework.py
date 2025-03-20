from .test_data import TestRunSpec, TestInfo
from .test_parser import parse_tests
from .test_runner import run_test
from .test_log import TestLog
from .test_utils import print_red, print_green

import logging


def _display_tests(tests: list[TestInfo]) -> None:
    print(f"Gathered {len(tests)} tests:")

    for test in tests:
        print(f"  {test.test_name}")


def test_framework_run(spec: TestRunSpec) -> None:
    file_logger = TestLog()
    file_logger.setup_logging()

    logging.info("Test framework started...")
    logging.info("Parsing tests from alkos...")
    tests_to_run = parse_tests(spec, file_logger)
    logging.info(f"Gathered {len(tests_to_run)} tests: {[test.test_name for test in tests_to_run]}")
    failed_tests = list[TestInfo]()

    # If specified display list of filtered tests and die
    if spec.display_tests_only:
        _display_tests(tests_to_run)
        logging.info("Displaying tests only run -> exiting...")
        return

    # Run the tests
    logging.info("Running tests...")
    for test in tests_to_run:
        logging.info(f"Running test {test.test_name}")
        result = run_test(spec.alkos_path, test, file_logger)

        if not result:
            logging.info(f"Failed test {test.test_name}")
            failed_tests.append(test)

    logging.info("Finished running tests.")
    if failed_tests:

        # Display test summary
        print_red(f"[FAILED] {len(failed_tests)} failed tests:")
        for test in failed_tests:
            print_red(f"  {test.test_name}")

        # Create a concatenated log of failed tests
        file_logger.save_concatenated_failed_tests_logs(failed_tests)

        exit(1)
    else:
        print_green(f"[PASSED] All tests ({len(tests_to_run)}) passed")
