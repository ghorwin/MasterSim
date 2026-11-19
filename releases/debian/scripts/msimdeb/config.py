"""Static configuration of the MasterSim Debian/Launchpad release process."""

import json
from pathlib import Path

PACKAGE = "mastersim"

# releases/debian
DEBIAN_RELEASE_DIR = Path(__file__).resolve().parents[2]
# repository root
REPO_DIR = DEBIAN_RELEASE_DIR.parents[1]
# debian/ packaging template (shared by all Ubuntu series)
TEMPLATE_DIR = DEBIAN_RELEASE_DIR / "debian"
# all generated files (tarball, source trees, packages) - ignored by git
WORK_DIR = DEBIAN_RELEASE_DIR / "work"
SERIES_FILE = DEBIAN_RELEASE_DIR / "series.json"

# file in the repository holding the version number (LONG_VERSION)
CONSTANTS_FILE = "MasterSim/src/MSIM_Constants.cpp"

# Launchpad PPA (anonymous FTP upload, same as 'dput ppa:ghorwin/sim')
PPA_HOST = "ppa.launchpad.net"
PPA_INCOMING = "~ghorwin/ubuntu/sim"

# Paths (relative to the repository root) that are not shipped in the
# upstream source archive. Directories end with '/'.
TARBALL_EXCLUDE_PATHS = (
	".github/",
	"bin/",
	"cross-check/",
	"data/examples/darwin64/",
	"data/examples/win64/",
	"data/misc/",
	"data/tests/",
	"doc/",
	"externals/zlib/",
	"lib_x64/",
	"releases/",
	"third-party/",
)

# Pre-compiled binaries and generated documents are never shipped
# (lintian: source-contains-prebuilt-binary / source-is-missing).
TARBALL_EXCLUDE_SUFFIXES = (
	".a", ".dll", ".dylib", ".exe", ".fmu", ".lib", ".pdf", ".so",
)


def load_series():
	"""Returns the dict {codename: version} of Ubuntu series to build for."""
	with open(SERIES_FILE, encoding="utf-8") as f:
		series = json.load(f)
	if not series:
		raise RuntimeError(f"No Ubuntu series defined in {SERIES_FILE}")
	return series
