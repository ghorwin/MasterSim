#!/bin/bash

# This script is executed as part of the build to create the test/example FMUs
# on Linux and macOS.
#
# Syntax: generate_FMUs.sh <path/to/cmake_library dir>
#
# Example library location: <path/to/cmake_library dir>/libMath003Part1.so  (i.e. <cmake binary dir>/lib)
#
# The FMUs are created in TestFMUs/fmus and copied into the matching test case
# directories in data/tests/<platform> (if they exist).
#
# Platform directories inside the FMU (FMI 1.0/2.0):
#   Linux: binaries/linux64/<modelIdentifier>.so
#   macOS: binaries/darwin64/<modelIdentifier>.dylib
#
# On macOS, darwin64 does not specify the CPU architecture. The dylibs should be
# universal binaries (arm64 + x86_64, see MSIM_UNIVERSAL_BINARIES in CMakeLists.txt),
# so that the FMUs work on Apple Silicon and Intel Macs. The script prints the
# contained architectures and warns if a library is not universal.

if [ "$#" -ne 1 ]; then
	echo "Syntax: generate_FMUs.sh <path/to/cmake_library dir>"
	exit 1
fi

# resolve library dir to absolute path before changing the working directory
LIB_DIR="$(cd -- "$1" && pwd)" || { echo "ERROR: library directory '$1' not found" >&2; exit 1; }
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR" || exit 1

if [[ "$(uname)" = "Darwin" ]]; then
	PLATFORM=darwin64
	LIB_EXT=dylib
else
	PLATFORM=linux64
	LIB_EXT=so
fi

# archive tool: 7za (as previously used on Linux), otherwise zip (always available on macOS)
if command -v 7za >/dev/null 2>&1; then
	ZIP_TOOL=7za
elif command -v zip >/dev/null 2>&1; then
	ZIP_TOOL=zip
else
	echo "ERROR: neither 7za nor zip found" >&2
	exit 1
fi

# Creates an FMU
#   $1 - FMU name = model identifier (name of shared library inside FMU)
#   $2 - name of CMake library target = source directory in TestFMUs
create_fmu() {
	local FMU_NAME=$1
	local LIB_NAME=$2
	local LIB_FILE="$LIB_DIR/lib$LIB_NAME.$LIB_EXT"

	if [ ! -f "$LIB_FILE" ]; then
		echo "ERROR: shared library '$LIB_FILE' not found" >&2
		return 1
	fi

	if [[ "$PLATFORM" = "darwin64" ]]; then
		local ARCHS
		ARCHS="$(lipo -archs "$LIB_FILE")"
		if [[ "$ARCHS" != *arm64* || "$ARCHS" != *x86_64* ]]; then
			echo "WARNING: $LIB_FILE is not a universal binary (architectures: $ARCHS)"
		fi
	fi

	rm -rf "$FMU_NAME" "$FMU_NAME.fmu" "$FMU_NAME.zip" &&
	mkdir -p "$FMU_NAME/binaries/$PLATFORM" &&
	cp "$LIB_FILE" "$FMU_NAME/binaries/$PLATFORM/$FMU_NAME.$LIB_EXT" &&
	cp "../$LIB_NAME/data/modelDescription.xml" "$FMU_NAME/" || return 1

	if [[ "$ZIP_TOOL" = "7za" ]]; then
		(cd "$FMU_NAME" && 7za a "../$FMU_NAME.zip" . > /dev/null) || return 1
	else
		(cd "$FMU_NAME" && zip -qrX "../$FMU_NAME.zip" .) || return 1
	fi
	mv "$FMU_NAME.zip" "$FMU_NAME.fmu" &&
	echo "Created $FMU_NAME.fmu ${ARCHS:+($ARCHS)}"
}

# Copies an FMU into a test case directory, if that exists
# (in Debian deployment mode the test directories are omitted)
#   $1 - FMU name
#   $2 - test case directory, relative to data/tests/<platform>
copy_to_test() {
	local TARGET_DIR="../../data/tests/$PLATFORM/$2/fmus/IBK"
	if [ -d "$TARGET_DIR" ]; then
		cp "$1.fmu" "$TARGET_DIR/" || return 1
	fi
}

mkdir -p fmus && cd fmus || exit 1

create_fmu Part1             Math003Part1          &&
create_fmu Part2             Math003Part2          &&
create_fmu Part3             Math003Part3          &&
create_fmu Prey              LotkaVolterraPrey     &&
create_fmu Predator          LotkaVolterraPredator &&
create_fmu FourRealInputVars FourRealInputVars     &&
create_fmu Room              Room                  &&
create_fmu Radiator          Radiator              || exit 1

echo "Updating test FMUs in data/tests/$PLATFORM"
copy_to_test Part1             Math_003_control_loop &&
copy_to_test Part2             Math_003_control_loop &&
copy_to_test Part3             Math_003_control_loop &&
copy_to_test Prey              Lotka_Volterra_System &&
copy_to_test Predator          Lotka_Volterra_System &&
copy_to_test Prey              FileReaderSlave       &&
copy_to_test FourRealInputVars FileReaderSlave       &&
copy_to_test Room              Room_Radiator         &&
copy_to_test Radiator          Room_Radiator         || exit 1
