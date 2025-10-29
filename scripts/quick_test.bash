#!/bin/bash

# Quick test runner for AlkOS
# Usage examples:
#   ./scripts/quick_test.bash                          # Run all tests
#   ./scripts/quick_test.bash String                   # Run all StringTest_* tests
#   ./scripts/quick_test.bash String Strlen            # Run StringTest_Strlen
#   ./scripts/quick_test.bash --list String            # List all StringTest_* tests
#   ./scripts/quick_test.bash --verbose String         # Run StringTest_* with verbose output

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
TEST_SCRIPT="${SCRIPT_DIR}/actions/run_tests.bash"

# Parse options
VERBOSE=""
LIST_ONLY=""
FILTERS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--verbose)
            VERBOSE="--verbose"
            shift
            ;;
        -l|--list|--display)
            LIST_ONLY="--display"
            shift
            ;;
        -h|--help)
            echo "Quick test runner for AlkOS"
            echo ""
            echo "Usage:"
            echo "  $0 [OPTIONS] [TEST_PATTERN...]"
            echo ""
            echo "Options:"
            echo "  -v, --verbose     Enable verbose output"
            echo "  -l, --list        List matching tests without running"
            echo "  -h, --help        Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                          # Run all tests"
            echo "  $0 String                   # Run all StringTest_* tests"
            echo "  $0 String Strlen            # Run StringTest_Strlen"
            echo "  $0 --list String            # List all StringTest_* tests"
            echo "  $0 --verbose String         # Run StringTest_* with verbose output"
            echo ""
            echo "Test naming patterns:"
            echo "  - Simple test name: TestName"
            echo "  - Fixture test: TestClass_TestName"
            echo "  - Use wildcards: 'StringTest_*' or '*Test_Strlen'"
            exit 0
            ;;
        *)
            FILTERS+=("$1")
            shift
            ;;
    esac
done

# Build the command
CMD=("${TEST_SCRIPT}")

if [ ${#FILTERS[@]} -eq 0 ]; then
    # No filters, run all tests
    echo "Running all tests..."
elif [ ${#FILTERS[@]} -eq 1 ]; then
    # Single filter - could be a test class or full test name
    FILTER="${FILTERS[0]}"
    if [[ "$FILTER" == *"_"* ]]; then
        # Contains underscore, treat as full test name
        echo "Running test: ${FILTER}"
        CMD+=(--filter "${FILTER}")
    else
        # No underscore, treat as test class prefix
        echo "Running tests matching: ${FILTER}Test_*"
        CMD+=(--filter "${FILTER}Test_*")
    fi
elif [ ${#FILTERS[@]} -eq 2 ]; then
    # Two arguments - treat as TestClass and TestName
    TEST_CLASS="${FILTERS[0]}"
    TEST_NAME="${FILTERS[1]}"
    FULL_NAME="${TEST_CLASS}Test_${TEST_NAME}"
    echo "Running test: ${FULL_NAME}"
    CMD+=(--filter "${FULL_NAME}")
else
    # Multiple filters - treat each as a pattern
    echo "Running tests matching: ${FILTERS[*]}"
    CMD+=(--filter "${FILTERS[@]}")
fi

# Add optional flags
[ -n "$VERBOSE" ] && CMD+=("${VERBOSE}")
[ -n "$LIST_ONLY" ] && CMD+=("${LIST_ONLY}")

# Execute the command
echo "Executing: ${CMD[*]}"
echo ""
exec "${CMD[@]}"
