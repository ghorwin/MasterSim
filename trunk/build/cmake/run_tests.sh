#!/bin/bash

if [ -e ../../bin/release/MasterSimulatorUI.app ]; then
  python ../../scripts/TestSuite/run_tests.py -p ../../data/tests/darwin64 -s ../../bin/release/MasterSimulator -e msim $*
else
  python ../../scripts/TestSuite/run_tests.py -p ../../data/tests/linux64 -s ../../bin/release/MasterSimulator -e msim $*
fi
