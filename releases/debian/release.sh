#!/bin/bash
# Start wrapper for the MasterSim Debian release tool, see readme.md.
#   ./release.sh --help
exec python3 "$(dirname "$(readlink -f "$0")")/scripts/release.py" "$@"
