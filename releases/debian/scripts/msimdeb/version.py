"""Version numbers: upstream version, Debian revision and changelog handling."""

import email.utils
import re

from . import config
from .util import ReleaseError, run

_LONG_VERSION_RE = re.compile(r'const\s+char\s*\*\s*const\s+LONG_VERSION\s*=\s*"([^"]+)"')
_CHANGELOG_HEAD_RE = re.compile(
	r"^(?P<pkg>\S+) \((?P<version>[^)]+)\) (?P<dist>[^;]+); (?P<rest>.*)$")
_TRAILER_RE = re.compile(r"^ -- (?P<who>.+?)  (?P<date>.+)$")


def upstream_version_from_text(text, origin):
	m = _LONG_VERSION_RE.search(text)
	if not m:
		raise ReleaseError(f"LONG_VERSION not found in {origin}")
	return m.group(1)


def upstream_version_in_worktree():
	path = config.REPO_DIR / config.CONSTANTS_FILE
	return upstream_version_from_text(path.read_text(encoding="utf-8"), path)


def upstream_version_at_ref(ref):
	text = run(["git", "-C", config.REPO_DIR, "show", f"{ref}:{config.CONSTANTS_FILE}"],
			capture=True).decode("utf-8")
	return upstream_version_from_text(text, f"{ref}:{config.CONSTANTS_FILE}")


class ChangelogHead:
	"""The topmost entry of debian/changelog in the packaging template."""

	def __init__(self, path):
		self.lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
		m = _CHANGELOG_HEAD_RE.match(self.lines[0].rstrip("\n"))
		if not m:
			raise ReleaseError(f"Cannot parse first line of {path}: {self.lines[0]!r}")
		self.package = m.group("pkg")
		self.version = m.group("version")          # e.g. 0.9.8-1
		self.rest = m.group("rest")                 # e.g. urgency=medium
		if "-" not in self.version:
			raise ReleaseError(f"Version {self.version} in {path} lacks a Debian revision (e.g. -1)")
		self.upstream, self.revision = self.version.rsplit("-", 1)
		if ":" in self.upstream:
			raise ReleaseError("Epochs are not supported")
		for i, line in enumerate(self.lines[1:], start=1):
			if _TRAILER_RE.match(line.rstrip("\n")):
				self.trailer_index = i
				self.maintainer = _TRAILER_RE.match(line.rstrip("\n")).group("who")
				break
		else:
			raise ReleaseError(f"No trailer line (' -- name <email>  date') in {path}")


def series_version(head, series_number, ppa_revision):
	"""Debian version for one Ubuntu series, e.g. 0.9.8-1~ubuntu24.04~ppa1.

	The '~ubuntuXX.YY' suffix sorts by release number (unlike codenames, which
	wrap around the alphabet) and is lower than the plain Debian version.
	'~ppaN' (instead of '.N') avoids lintian's binary-nmu-debian-revision-in-source.
	"""
	return f"{head.version}~ubuntu{series_number}~ppa{ppa_revision}"


def render_series_changelog(head, codename, version):
	"""Returns the changelog text with the top entry targeted at one series."""
	lines = list(head.lines)
	lines[0] = f"{head.package} ({version}) {codename}; {head.rest}\n"
	date = email.utils.format_datetime(email.utils.localtime())
	lines[head.trailer_index] = f" -- {head.maintainer}  {date}\n"
	return "".join(lines)
