#!/bin/bash
# Compiles the translation files (*.ts) into binary *.qm files.

# find a tool: first the plain name, then the "-qt6" suffixed variant (e.g. Fedora packages)
find_tool() {
	for name in "$1" "$1-qt6"; do
		if command -v "$name" > /dev/null 2>&1; then
			echo "$name"
			return 0
		fi
	done
	echo "ERROR: Neither '$1' nor '$1-qt6' found in PATH." >&2
	return 1
}

cd "$(dirname "$0")" || exit 1

LRELEASE=$(find_tool lrelease) || exit 1
"$LRELEASE" MasterSimulatorUI_de.ts -qm mastersim-gui_de.qm
