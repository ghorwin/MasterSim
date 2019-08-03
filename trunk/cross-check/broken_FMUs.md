# Broken/invalid FMUs

Below is a list of test FMUs that either do not work as expected, crash or throw errors.


## Missing Licenses

- Adams

No fix possible (no license provided).


## Not allowing overstepping

FMUs abort last communication step, even if it exceeds end time (due to rounding errors) only
by a small time fraction.

- MapleSim
- 20Sim

Fix: shorten last communication interval to hit end time point exactly.

