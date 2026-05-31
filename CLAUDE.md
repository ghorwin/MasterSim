# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## About

MasterSim is an FMI (Functional Mock-Up Interface) Co-Simulation master and library. It supports FMI for Co-Simulation versions 1.0 and 2.0, with multiple iteration algorithms (Gauss-Jacobi, Gauss-Seidel, Newton) and error-controlled time stepping.

## Build System

The project uses CMake. All build scripts live in `build/` and must be run from that directory.

```bash
# Full build (solver + Qt GUI)
cd build && ./build.sh

# Solver only (no Qt dependency)
cd build && ./build_only_solver.sh

# Common options: debug | release | reldeb | no-gui | verbose | deploy
cd build && ./build.sh debug no-gui
cd build && ./build.sh release
```

Build artifacts are copied to `bin/release/` after a successful build:
- `bin/release/mastersim` — CLI solver
- `bin/release/mastersim-gui` — Qt GUI

The CMake build directory is named `bb-gcc-qt-debug` (or similar) inside `build/`; don't edit files there.

**Qt requirement:** Qt 6 is required. The build script auto-detects Qt installed via `aqtinstall` at `~/Qt/${AQT_QT_VERSION}/gcc_64` (default version 6.9.3). Override with `AQT_QT_VERSION` / `AQT_QT_PREFIX` env vars. Use `no-gui` to skip Qt and build only the solver and test FMUs.

## Running Tests

```bash
# Run full regression suite (called automatically by build.sh unless skip-test is passed)
cd build && ./run_tests.sh

# Run tests directly against an already-built solver
python3 scripts/TestSuite/run_tests.py \
    -p data/tests/linux64 \
    -s bin/release/mastersim \
    -e msim

# Init-only mode: faster check that each case starts without crashing
python3 scripts/TestSuite/run_tests.py \
    -p data/tests/linux64 -s bin/release/mastersim -e msim --test-init

# Run all cases including those without reference results (to generate new refs)
python3 scripts/TestSuite/run_tests.py \
    -p data/tests/linux64 -s bin/release/mastersim -e msim --run-all
```

Test cases live in `data/tests/linux64/` (one subdirectory per scenario). Reference results are stored alongside each test case and compared by the runner.

## Architecture

### Module layout

| Directory | Purpose |
|-----------|---------|
| `MasterSim/src/` | Core library (`libMasterSim`) — all simulation logic |
| `MasterSimulator/src/` | CLI executable (`mastersim`) — thin wrapper around the library |
| `MasterSimulatorUI/src/` | Qt6 GUI executable (`mastersim-gui`) |
| `externals/IBK/` | IBK utility library (paths, units, parameters, logging) |
| `externals/IBKMK/` | IBK Math Kernel subset |
| `externals/TiCPP/` | TinyXML with IBK extensions — XML parsing |
| `externals/BlockMod/` | Block-diagram schematic widget (GUI only) |
| `externals/minizip/` | Zip extraction for FMU archives |
| `externals/zlib/` | zlib (Windows only; Linux/Mac use system zlib) |
| `TestFMUs/` | Source for test FMUs; compiled to `.fmu` files by `TestFMUs/generate_FMUs.sh` |
| `data/tests/linux64/` | Regression test cases and reference results |
| `data/examples/` | Example `.msim` project files |
| `scripts/TestSuite/` | Python-based regression test runner |

### Core library (`MasterSim/`)

All classes use the `MASTER_SIM` namespace and the `MSIM_` filename/class prefix.

Key classes:
- **`MSIM_Project`** — Reads and stores `.msim` project files; defines `SimulatorDef`, `ConnectionDef`, master mode (`MM_GAUSS_JACOBI`, `MM_GAUSS_SEIDEL`, `MM_NEWTON`), and error control mode.
- **`MSIM_MasterSim`** — Central orchestrator: `importFMUs()` → `initialize()` → `simulate()` → `storeState()`/`restoreState()`. Used by both CLI and GUI.
- **`MSIM_FMUManager`** — Loads and caches FMU shared libraries; manages extraction from `.fmu` zip archives.
- **`MSIM_FMUSlave`** — Wraps a single FMU instance; implements both FMI 1.0 and 2.0 C function calls.
- **`MSIM_AbstractAlgorithm`** / `AlgorithmGaussJacobi`, `AlgorithmGaussSeidel`, `AlgorithmNewton` — Master algorithm implementations.
- **`MSIM_OutputWriter`** — Writes simulation results to DataIO-format binary files.
- **`MSIM_ModelDescription`** — Parses `modelDescription.xml` from inside an FMU.

### GUI (`MasterSimulatorUI/`)

Qt6 widgets application. Main views (each a `QWidget` subclass):
- `MSIMViewSlaves` — Add/configure FMU slaves
- `MSIMViewConnections` — Wire inputs/outputs between slaves
- `MSIMViewSimulation` — Set master parameters and launch simulation

The network schematic uses the `BlockMod` library. Undo/redo is implemented via `MSIMUndo*` command classes.

### Project file format

`.msim` files are plain-text (key-value + structured sections), parsed by `MSIM_Project`. The `.bm` sidecar stores the graphical block-diagram layout used by the GUI.

## Coding conventions

- C++11 minimum for the core library; C++17 for Qt-dependent code.
- All library source files are prefixed `MSIM_`; GUI files use `MSIM` (no underscore).
- IBK assertion macros (`IBK_ASSERT`) are used throughout instead of standard `assert`.
- Errors are reported by throwing `IBK::Exception`; callers catch and display them.
