#!/bin/bash

if [ -e ../../bin/release/MasterSim.app ]; then
  python ../../scripts/TestSuite/run_tests.py -p ../../data/tests/darwin64 -s ../../bin/release/mastersim -e msim $*
else
  python3 ../../scripts/TestSuite/run_tests.py -p ../../data/tests/linux64 -s ../../bin/release/mastersim -e msim $*
fi
