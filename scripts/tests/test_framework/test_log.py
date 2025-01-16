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
        self._rep = 0

    def save_init_log(self, output: str) -> None:
        with open(self._dir / "init.log", 'w') as f:
            f.write(output)

    def save_log(self, info: TestInfo, output: str) -> None:
        with open(self._dir / f"test_{self._rep}_{info.test_name}.log", 'w') as f:
            f.write(output)

        self._rep += 1

    @contextmanager
    def save_log_with_context(self, info: TestInfo):
        file_path = self._dir / f"test_{self._rep}_{info.test_name}.log"

        file = open(file_path, 'w')
        try:
            yield file
        finally:
            self._rep += 1
            file.close()

    def setup_logging(self) -> None:
        logging.basicConfig(level=logging.DEBUG, format="%(asctime)s - %(levelname)s - %(message)s",
                            filename=self._dir / 'exec_log.log', )
