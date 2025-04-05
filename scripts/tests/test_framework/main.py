#!/bin/python

import sys
import traceback

from .test_cli import process_args
from .test_framework import test_framework_run


def main(args: list[str]) -> None:
    try:
        test_spec = process_args(args)
        test_framework_run(test_spec)
    except Exception as e:
        print(f"Failed to run tests. Cause of the problem: {e}")
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main(sys.argv[1:])
