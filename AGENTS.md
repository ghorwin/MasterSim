# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## About

MasterSim is an FMI (Functional Mock-Up Interface) Co-Simulation master and library. It supports FMI for Co-Simulation versions 1.0 and 2.0, with multiple iteration algorithms (Gauss-Jacobi, Gauss-Seidel, Newton) and error-controlled time stepping.

## Build System

The project uses CMake (target-based; qmake `.pro` files are kept in sync for QtCreator). The build scripts live in `build/` and can be run from any directory.

```bash
# Full build (solver + Qt GUI), release mode by default
build/build.sh

# Solver only (no Qt dependency)
build/build_only_solver.sh

# Options: debug | release | reldeb | <cpu count> | no-gui | verbose | deployment (alias: deploy)
#          | nightly | omp | gprof | skip-test   (unknown options abort the script)
build/build.sh debug no-gui
build/build.sh release skip-test
```

Common CMake settings (C++17, output directories, rpath, `DEPLOYMENT`/`USE_OMP`/`DISABLE_QT` options, Qt AUTOMOC/UIC/RCC) live in `externals/IBK/build.cmake`, which is included by the top-level `CMakeLists.txt`. That file is shared verbatim with other IBK projects, so put MasterSim-specific settings in the top-level `CMakeLists.txt` instead. Libraries are CMake targets that export their include paths/dependencies (`target_link_libraries(... PUBLIC ...)`), so do not use `include_directories()`. zlib comes from `find_package(ZLIB)` (system zlib on Linux/macOS, vcpkg on Windows: triplet `x64-windows` for MSVC, `x64-mingw-static` for MinGW; both build scripts install it via `build/setup_zlib_vcpkg.bat` if missing).

**macOS:** everything (solver, GUI, test FMUs) is built as a universal binary (`arm64;x86_64`, option `MSIM_UNIVERSAL_BINARIES`, qmake: `.qmake.conf`), because FMI 1.0/2.0 only has `binaries/darwin64`. x86_64-only FMUs run on Apple Silicon via `arch -x86_64 bin/release/mastersim`. `-ffp-contract=off` keeps results identical across architectures (needed to match the test references).

Executables are written directly to `bin/release/` (or `bin/debug/` for debug builds):
- `bin/release/mastersim` — CLI solver
- `bin/release/mastersim-gui` — Qt GUI

Libraries (including the test FMU shared libraries) go to `build/bb-<...>/lib`. The CMake build directory is named `bb-gcc-qt-release` (or similar) inside `build/`; don't edit files there.

The qmake build (`MasterSim-session.pro`) uses `externals/IBK/IBK.pri`: libraries go to `<shadow-build-dir>/lib` and executables to `bin/<debug|release>` (or to the shadow dir with `OPTIONS += binaries_to_shadow_dir` in `externals/IBK/CONFIG.pri`).

**Qt requirement:** Qt 6 is required. The build script auto-detects Qt installed via `aqtinstall` at `~/Qt/${AQT_QT_VERSION}/gcc_64` (default version 6.9.3). Override with `AQT_QT_VERSION` / `AQT_QT_PREFIX` env vars. Use `no-gui` to skip Qt and build only the solver and test FMUs.

## Agents

- never automatically build

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
| `externals/zlib/` | zlib sources (only used by the qmake build; CMake uses system/vcpkg zlib) |
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
