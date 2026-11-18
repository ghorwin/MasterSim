#!/bin/bash

# Runs the regression test suite against bin/release/mastersim.
# Additional arguments are passed to run_tests.py (e.g. --test-init or --run-all).

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"

if [[ "$(uname)" = "Darwin" ]]; then
  TEST_DIR=$ROOT_DIR/data/tests/darwin64
else
  TEST_DIR=$ROOT_DIR/data/tests/linux64
fi

python3 $ROOT_DIR/scripts/TestSuite/run_tests.py -p $TEST_DIR -s $ROOT_DIR/bin/release/mastersim -e msim "$@"
