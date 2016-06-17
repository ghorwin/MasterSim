#!/bin/bash

cd ../../data/tests
../../bin/release/MasterSimulator init_test_linux64.cfg -t --verbosity-level=3
cd -

#python ../../scripts/tests/run_tests.py -p ../../data/tests -s ../../bin/release/MasterSimulator -e sim $*


