#!/bin/bash
# Creates the Phantom style comparison page (thin wrapper around the Python scripts).
#
# Usage: ./create_comparison.sh [path-to-SIM-VICUS-checkout]
#
# Work files (variant sources, build) go to ./work, the page to ./phatom-fork-comparison.
# The VICUS variant needs a local SIM-VICUS checkout (private repository); without it,
# the VICUS variant is skipped.

set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
WORK="$HERE/work"
OUT="$HERE/phatom-fork-comparison"

VICUS_ARG=()
if [ -n "$1" ]; then VICUS_ARG=(--vicus "$1"); fi

python3 "$HERE/fetch_variants.py" "$WORK/variants" "${VICUS_ARG[@]}"
python3 "$HERE/render_shots.py" "$WORK/variants" "$WORK/build" "$OUT"
python3 "$HERE/make_page.py" "$OUT"
