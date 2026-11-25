#!/bin/bash
# Updates the translation files (*.ts) from the sources and opens them in Qt Linguist.

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

LUPDATE=$(find_tool lupdate) || exit 1
"$LUPDATE" ../../MasterSimulatorUI.pro || exit 1

LINGUIST=$(find_tool linguist) || exit 1
"$LINGUIST" MasterSimulatorUI_de.ts
