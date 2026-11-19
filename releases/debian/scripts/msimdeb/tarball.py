"""Creates the upstream source archive mastersim_<version>.orig.tar.xz.

The archive is generated with 'git archive' from a git ref (normally the
release tag v<version>), so it does not depend on the state of the working
tree. Files listed in config.TARBALL_EXCLUDE_* are dropped. The result is
reproducible: the same ref always yields a byte-identical archive. This
matters because all Ubuntu series in the PPA share one orig tarball.
"""

import io
import tarfile

from . import config, version
from .util import ReleaseError, info, run, warn


def orig_tarball_path(upstream):
	return config.WORK_DIR / f"{config.PACKAGE}_{upstream}.orig.tar.xz"


def default_ref():
	return f"v{version.upstream_version_in_worktree()}"


def _ref_exists(ref):
	return run(["git", "-C", config.REPO_DIR, "rev-parse", "--verify", "--quiet", f"{ref}^{{commit}}"],
			capture=True, check=False) != b""


def _is_excluded(rel_path, is_dir):
	check = rel_path + "/" if is_dir else rel_path
	if any(check.startswith(p) for p in config.TARBALL_EXCLUDE_PATHS):
		return True
	return not is_dir and rel_path.lower().endswith(config.TARBALL_EXCLUDE_SUFFIXES)


def _build_archive(ref, upstream):
	prefix = f"{config.PACKAGE}-{upstream}/"
	raw = run(["git", "-C", config.REPO_DIR, "archive", "--format=tar", f"--prefix={prefix}", ref],
			capture=True)
	out = io.BytesIO()
	kept = dropped = 0
	with tarfile.open(fileobj=io.BytesIO(raw), mode="r:") as src, \
		tarfile.open(fileobj=out, mode="w:xz", format=tarfile.PAX_FORMAT, preset=9) as dst:
		for member in src:
			rel = member.name[len(prefix):] if member.name.startswith(prefix) else member.name
			if rel and _is_excluded(rel.rstrip("/"), member.isdir()):
				dropped += 1
				continue
			# normalize ownership so the archive does not depend on the local user
			member.uid = member.gid = 0
			member.uname = member.gname = "root"
			dst.addfile(member, src.extractfile(member) if member.isfile() else None)
			kept += 1
	info(f"Archive contains {kept} entries, {dropped} excluded")
	return out.getvalue()


def create(ref=None, force=False):
	"""Creates the orig tarball, returns (path, upstream version)."""
	ref = ref or default_ref()
	if ref == "worktree":
		# commit object of the uncommitted changes of tracked files (does not
		# touch the working tree or the stash list); empty if nothing changed
		stash = run(["git", "-C", config.REPO_DIR, "stash", "create"], capture=True).decode().strip()
		ref = stash or "HEAD"
		warn(f"Using uncommitted working tree state ({ref}); untracked files are not included.")
	if not _ref_exists(ref):
		raise ReleaseError(
			f"git ref '{ref}' does not exist. Tag the release first "
			f"(git tag {ref}) or pass --ref HEAD for a test build.")
	if not ref.startswith("v"):
		warn(f"Building from '{ref}' instead of a release tag - use this for test builds only.")

	upstream = version.upstream_version_at_ref(ref)
	head = version.ChangelogHead(config.TEMPLATE_DIR / "changelog")
	if head.upstream != upstream:
		raise ReleaseError(
			f"Upstream version {upstream} (from {ref}) does not match debian/changelog "
			f"version {head.version}. Add a changelog entry for {upstream}-1 first.")

	info(f"Creating source archive for {config.PACKAGE} {upstream} from '{ref}'")
	data = _build_archive(ref, upstream)
	target = orig_tarball_path(upstream)
	config.WORK_DIR.mkdir(exist_ok=True)
	if target.exists():
		if target.read_bytes() == data:
			info(f"{target.name} is up to date (identical content)")
			return target, upstream
		if not force:
			raise ReleaseError(
				f"{target} exists with different content. If this version was already "
				"uploaded to Launchpad, the tarball must not change (Launchpad rejects it). "
				"Use --force to overwrite anyway.")
		warn(f"Overwriting {target}")
	target.write_bytes(data)
	info(f"Created {target} ({len(data) / 1e6:.1f} MB)")
	return target, upstream


def existing(upstream):
	path = orig_tarball_path(upstream)
	if not path.exists():
		raise ReleaseError(f"{path} missing - run the 'tarball' step first")
	return path
