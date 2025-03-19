from contextlib import contextmanager
from pathlib import Path
from datetime import datetime
from .test_data import TestInfo
import logging

SCRIPT_DIRECTORY = Path(__file__).parent.resolve()


class TestLog:
    def __init__(self) -> None:
        current_time = datetime.now()

        self._dir = SCRIPT_DIRECTORY / ".." / "test_framework_logs" / current_time.strftime("%Y_%m_%d_%H_%M_%S")
        self._dir.mkdir(parents=True, exist_ok=True)

    def save_log(self, info: TestInfo, output: str) -> None:
        with open(self.get_log_file_path(info), 'w') as f:
            f.write(output)

    @contextmanager
    def save_log_with_context(self, info: TestInfo):
        file_path = self.get_log_file_path(info)

        file = open(file_path, 'w')
        try:
            yield file
        finally:
            file.close()

    def save_concatenated_failed_tests_logs(self, failed_tests: list[TestInfo]) -> None:
        with open(self._dir / "failed_tests.log", 'w') as f:
            for test in failed_tests:
                with open(self.get_log_file_path(test), 'r') as test_log:
                    f.write(f"{"="*80}\n")
                    f.write(f"Test: {test.test_name}\n\n\n")
                    f.write(test_log.read())
                    f.write(f"{"="*80}\n")
                    f.write("\n\n")

    def get_init_log_path(self) -> Path:
        return Path(self._dir / "init.log")

    def get_log_file_path(self, info: TestInfo) -> Path:
        return Path(self._dir / f"{info.test_name}.log")

    def setup_logging(self) -> None:
        logging.basicConfig(level=logging.DEBUG, format="%(asctime)s - %(levelname)s - %(message)s",
                            filename=self._dir / 'exec_log.log', )
