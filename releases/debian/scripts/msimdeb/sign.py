"""Signs .dsc and .changes with gpg on the host (replacement for debsign).

The packages are built unsigned inside the container, so the private key
never has to be copied anywhere. Signing works like debsign:
  1. clearsign the .dsc
  2. update the .dsc checksums in the .buildinfo, clearsign the .buildinfo
  3. update the .dsc/.buildinfo checksums in the .changes, clearsign it
"""

import hashlib
import os
import re
import tempfile
from pathlib import Path

from .util import ReleaseError, info, run

_SIG_BEGIN = "-----BEGIN PGP SIGNED MESSAGE-----"
_HASHES = {"Files": "md5", "Checksums-Md5": "md5", "Checksums-Sha1": "sha1", "Checksums-Sha256": "sha256"}


def _strip_signature(text):
	"""Returns the signed payload of a clearsigned file (or the text unchanged)."""
	if not text.startswith(_SIG_BEGIN):
		return text
	# header lines ('Hash: ...') end at the first empty line
	body = text.split("\n\n", 1)[1]
	body = body.split("\n-----BEGIN PGP SIGNATURE-----", 1)[0]
	# undo dash-escaping
	return "\n".join(line[2:] if line.startswith("- ") else line for line in body.split("\n")) + "\n"


def _clearsign(path, key):
	payload = _strip_signature(path.read_text(encoding="utf-8"))
	with tempfile.TemporaryDirectory() as tmp:
		unsigned = Path(tmp) / path.name
		signed = Path(tmp) / (path.name + ".asc")
		unsigned.write_text(payload, encoding="utf-8")
		cmd = ["gpg", "--armor", "--clearsign", "--output", signed]
		if key:
			cmd += ["--local-user", key]
		run(cmd + [unsigned])
		path.write_bytes(signed.read_bytes())


def _update_checksums(target_path, file_paths):
	"""Updates size and checksums of the given files listed in target_path."""
	data = {p.name: p.read_bytes() for p in file_paths}
	text = _strip_signature(target_path.read_text(encoding="utf-8"))
	field = None
	out = []
	for line in text.split("\n"):
		m = re.match(r"^(\S+):", line)
		if m:
			field = m.group(1)
		elif field in _HASHES and line.startswith(" ") and line.split()[-1] in data:
			parts = line.split()
			content = data[parts[-1]]
			parts[0] = hashlib.new(_HASHES[field], content).hexdigest()
			parts[1] = str(len(content))
			line = " " + " ".join(parts)
		out.append(line)
	target_path.write_text("\n".join(out), encoding="utf-8")


def sign(build, key=None):
	key = key or os.environ.get("DEBSIGN_KEYID")
	dsc, changes = build.file(".dsc"), build.source_changes
	buildinfo = build.file("_source.buildinfo")
	for p in (dsc, changes):
		if not p.exists():
			raise ReleaseError(f"{p} missing - run the 'source' step first")
	info(f"[{build.codename}] Signing {build.version}" + (f" with key {key}" if key else ""))
	_clearsign(dsc, key)
	signed = [dsc]
	if buildinfo.exists():
		_update_checksums(buildinfo, [dsc])
		_clearsign(buildinfo, key)
		signed.append(buildinfo)
	_update_checksums(changes, signed)
	_clearsign(changes, key)


def is_signed(path):
	return path.read_text(encoding="utf-8").startswith(_SIG_BEGIN)
