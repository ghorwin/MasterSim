"""Builds binary (test) and source packages for one Ubuntu series."""

import shutil
import tarfile

from . import config, container, tarball, version
from .util import info

_LINTIAN = "lintian --fail-on error --info --pedantic --show-overrides --color=auto"


class SeriesBuild:
	"""Paths and versions of one package build: work/<codename>/<kind>/."""

	def __init__(self, codename, number, kind, ppa_revision):
		self.codename = codename
		self.number = number
		self.kind = kind    # 'binary' or 'source'
		self.head = version.ChangelogHead(config.TEMPLATE_DIR / "changelog")
		self.upstream = self.head.upstream
		self.version = version.series_version(self.head, number, ppa_revision)
		self.dir = config.WORK_DIR / codename / kind
		self.tree_name = f"{config.PACKAGE}-{self.upstream}"

	def file(self, suffix):
		"""E.g. file('.dsc') -> work/noble/source/mastersim_0.9.8-1~ubuntu24.04.1.dsc"""
		return self.dir / f"{config.PACKAGE}_{self.version}{suffix}"

	@property
	def source_changes(self):
		return self.file("_source.changes")

	def prepare(self):
		"""Creates a fresh source tree: orig tarball + debian/ template + series changelog."""
		orig = tarball.existing(self.upstream)
		info(f"[{self.codename}] Preparing {self.kind} build of {self.version} in {self.dir}")
		if self.dir.exists():
			shutil.rmtree(self.dir)
		self.dir.mkdir(parents=True)
		shutil.copy2(orig, self.dir / orig.name)
		with tarfile.open(orig) as tf:
			tf.extractall(self.dir, filter="data")
		tree = self.dir / self.tree_name
		if not tree.is_dir():
			raise RuntimeError(f"{orig.name} does not contain {self.tree_name}/")
		shutil.copytree(config.TEMPLATE_DIR, tree / "debian")
		(tree / "debian" / "changelog").write_text(
			version.render_series_changelog(self.head, self.codename, self.version), encoding="utf-8")

	def _run(self, script):
		container.run_in(self.codename, self.dir, f"/work/{self.tree_name}", script)


def build_binary(codename, number, ppa_revision):
	"""Full binary build in the container, followed by lintian."""
	b = SeriesBuild(codename, number, "binary", ppa_revision)
	container.build_image(codename, number)
	b.prepare()
	b._run(f"dpkg-buildpackage -b -us -uc && {_LINTIAN} ../{b.file('_amd64.changes').name}")
	debs = sorted(p.name for p in b.dir.glob("*.deb"))
	info(f"[{codename}] Binary build OK: {', '.join(debs)}")
	return b


def build_source(codename, number, ppa_revision):
	"""Unsigned source package (always including the orig tarball), then lintian."""
	b = SeriesBuild(codename, number, "source", ppa_revision)
	container.build_image(codename, number)
	b.prepare()
	b._run(f"dpkg-buildpackage -S -sa -us -uc && {_LINTIAN} ../{b.source_changes.name}")
	info(f"[{codename}] Source package OK: {b.source_changes.name}")
	return b
