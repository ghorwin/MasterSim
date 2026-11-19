"""Runs Debian tooling inside Ubuntu containers (podman or docker).

One image per Ubuntu series holds the packaging tools and all build
dependencies from debian/control. Image builds are cached by the container
engine and are only redone when debian/control changes.
"""

import os
import shutil

from . import config
from .util import ReleaseError, info, run

_CONTAINERFILE = """\
FROM docker.io/library/ubuntu:{number}
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \\
 && apt-get install -y --no-install-recommends \\
      build-essential devscripts equivs fakeroot lintian
COPY control /tmp/control
RUN cd /tmp \\
 && mk-build-deps --install --remove \\
      --tool "apt-get -y --no-install-recommends" /tmp/control \\
 && rm -rf /var/lib/apt/lists/*
"""


def engine():
	name = os.environ.get("CONTAINER_ENGINE")
	candidates = [name] if name else ["podman", "docker"]
	for c in candidates:
		if shutil.which(c):
			return c
	raise ReleaseError("Neither podman nor docker found (or set CONTAINER_ENGINE)")


def _engine_cmd():
	"""Container engine plus global options.

	Rootless podman without a fully working systemd user session (e.g. after
	'su', or in an ssh session without lingering) fails with 'sd-bus call:
	Access denied'. Package builds need no cgroup resource limits, so podman
	always uses the plain cgroupfs manager.
	"""
	eng = engine()
	cmd = [eng]
	if os.path.basename(eng) == "podman":
		cmd.append("--cgroup-manager=cgroupfs")
	return cmd


def image_name(codename):
	return f"{config.PACKAGE}-deb:{codename}"


def build_image(codename, number):
	info(f"Preparing build container for Ubuntu {number} ({codename})")
	context = config.WORK_DIR / "container" / codename
	context.mkdir(parents=True, exist_ok=True)
	shutil.copy2(config.TEMPLATE_DIR / "control", context / "control")
	(context / "Containerfile").write_text(_CONTAINERFILE.format(number=number), encoding="utf-8")
	run(_engine_cmd() + ["build", "--tag", image_name(codename), "--file", context / "Containerfile", context])


def run_in(codename, host_dir, workdir, script):
	"""Runs a bash script in the series container, host_dir mounted at /work."""
	cmd = _engine_cmd() + ["run", "--rm", "--volume", f"{host_dir}:/work:Z", "--workdir", workdir]
	if engine() == "docker":
		# make files written in the container belong to the calling user
		cmd += ["--user", f"{os.getuid()}:{os.getgid()}"]
	cmd += [image_name(codename), "bash", "-euo", "pipefail", "-c", script]
	run(cmd)
