"""Uploads a signed source package to the Launchpad PPA via anonymous FTP.

This is what 'dput ppa:ghorwin/sim <changes>' does: all files listed in the
.changes file are uploaded first, the .changes file itself last.
"""

import ftplib

from . import config, sign
from .util import ReleaseError, info


def files_in_changes(changes_path):
	names, in_files = [], False
	for line in sign._strip_signature(changes_path.read_text(encoding="utf-8")).split("\n"):
		if line.startswith("Files:"):
			in_files = True
		elif in_files and line.startswith(" "):
			names.append(line.split()[-1])
		elif in_files:
			break
	if not names:
		raise ReleaseError(f"No 'Files:' entries found in {changes_path}")
	return names


def upload(build, dry_run=False):
	changes = build.source_changes
	if not changes.exists():
		raise ReleaseError(f"{changes} missing - run the 'source' step first")
	if not sign.is_signed(changes):
		raise ReleaseError(f"{changes.name} is not signed - run the 'sign' step first")
	files = [build.dir / n for n in files_in_changes(changes)] + [changes]
	for f in files:
		if not f.exists():
			raise ReleaseError(f"{f} listed in {changes.name} but missing")

	target = f"ftp://{config.PPA_HOST}/{config.PPA_INCOMING}"
	if dry_run:
		info(f"[{build.codename}] Dry run - would upload to {target}:")
		for f in files:
			print(f"    {f.name}")
		return

	info(f"[{build.codename}] Uploading to {target}")
	with ftplib.FTP(config.PPA_HOST, timeout=120) as ftp:
		ftp.login()   # anonymous
		ftp.cwd(config.PPA_INCOMING)
		for f in files:
			print(f"    {f.name} ({f.stat().st_size / 1e6:.2f} MB)", flush=True)
			with open(f, "rb") as fh:
				ftp.storbinary(f"STOR {f.name}", fh)
	info(f"[{build.codename}] Upload done. Launchpad sends an accept/reject mail to the signer.")
