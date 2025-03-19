from dataclasses import dataclass
from enum import IntEnum

@dataclass
class TestRunSpec:
    alkos_path: str
    filters: list[str]
    blocks: list[str]
    display_tests_only: bool
    verbose: bool


class TestInfo:
    test_name: str

class TestState(IntEnum):
    UNKNOWN = 0
    SUCCESS = 1
    FAILURE = 2
    ERROR_DOUBLE_SUCCESS = 3
    ERROR_FAIL_AFTER_SUCCESS = 4
    ERROR_FAIL_AFTER_FAIL = 5
    ERROR_SUCCESS_AFTER_FAIL = 6

MAX_ALKOS_BOOT_TIME = 10
MAX_ALKOS_WAIT_SYNC_TIME = 10
MAX_ALKOS_TEST_TIME = 300
