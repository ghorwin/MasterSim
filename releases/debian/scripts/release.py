#!/usr/bin/env python3
"""MasterSim Debian/Ubuntu release tool (Launchpad PPA ppa:ghorwin/sim).

Steps (each can be run on its own, 'all' runs them in order):

  tarball   create work/mastersim_<ver>.orig.tar.xz from the git tag v<ver>
  build     test-build the binary package in an Ubuntu container + lintian
  source    build the source package in the container + lintian
  sign      sign .dsc/.changes with gpg on the host
  upload    upload the signed source package to Launchpad (asks first)
  all       tarball, build, source, sign, upload

The Ubuntu series are listed in series.json, the packaging template is the
debian/ directory next to it. See readme.md for the complete procedure.
"""

import argparse
import sys

from msimdeb import config, package, sign, tarball, upload
from msimdeb.util import ReleaseError, confirm, info

STEPS = ("tarball", "build", "source", "sign", "upload")


def parse_args(argv):
	p = argparse.ArgumentParser(
		description=__doc__.split("\n\n")[0],
		epilog="\n\n".join(__doc__.split("\n\n")[1:]),
		formatter_class=argparse.RawDescriptionHelpFormatter)
	p.add_argument("step", choices=STEPS + ("all",))
	p.add_argument("--series", help="comma-separated codenames (default: all in series.json)")
	p.add_argument("--ref", help="git ref for the tarball (default: tag v<LONG_VERSION>), "
					"e.g. HEAD, or 'worktree' for uncommitted changes (test builds only)")
	p.add_argument("--ppa-rev", type=int, default=1, metavar="N",
					help="PPA revision, increase for re-uploads of the same "
					"Debian version (…~ubuntu24.04~ppaN, default: 1)")
	p.add_argument("--key", help="gpg key id for signing (default: $DEBSIGN_KEYID or gpg default key)")
	p.add_argument("--force", action="store_true", help="overwrite an existing, different orig tarball")
	p.add_argument("--dry-run", action="store_true", help="upload: only list the files")
	p.add_argument("--yes", action="store_true", help="do not ask before uploading")
	return p.parse_args(argv)


def selected_series(arg):
	series = config.load_series()
	if not arg:
		return series
	wanted = [s.strip() for s in arg.split(",") if s.strip()]
	unknown = [s for s in wanted if s not in series]
	if unknown:
		raise ReleaseError(f"Unknown series {', '.join(unknown)} (known: {', '.join(series)})")
	return {s: series[s] for s in wanted}


def main(argv):
	args = parse_args(argv)
	series = selected_series(args.series)
	steps = STEPS if args.step == "all" else (args.step,)

	if "tarball" in steps:
		tarball.create(args.ref, args.force)

	if "build" in steps:
		for codename, number in series.items():
			package.build_binary(codename, number, args.ppa_rev)

	if "source" in steps:
		for codename, number in series.items():
			package.build_source(codename, number, args.ppa_rev)

	builds = [package.SeriesBuild(c, n, "source", args.ppa_rev) for c, n in series.items()]

	if "sign" in steps:
		for b in builds:
			sign.sign(b, args.key)

	if "upload" in steps:
		if not args.dry_run:
			names = ", ".join(f"{b.version} ({b.codename})" for b in builds)
			if not confirm(f"Upload {names} to Launchpad?", args.yes):
				info("Upload cancelled.")
				return 0
		for b in builds:
			upload.upload(b, args.dry_run)
	return 0


if __name__ == "__main__":
	try:
		sys.exit(main(sys.argv[1:]))
	except ReleaseError as e:
		print(f"ERROR: {e}", file=sys.stderr)
		sys.exit(1)
	except KeyboardInterrupt:
		print("\nAborted.", file=sys.stderr)
		sys.exit(130)
