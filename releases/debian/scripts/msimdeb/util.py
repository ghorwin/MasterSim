"""Small helpers shared by all release steps."""

import shlex
import subprocess
import sys


class ReleaseError(RuntimeError):
	"""Error that aborts the release process with a message (no traceback)."""


def info(msg):
	print(f"*** {msg}", flush=True)


def warn(msg):
	print(f"WARNING: {msg}", file=sys.stderr, flush=True)


def run(cmd, cwd=None, capture=False, input_bytes=None, check=True):
	"""Runs a command (list of arguments), echoing it first.

	With capture=True, stdout is returned as bytes.
	"""
	print("$ " + " ".join(shlex.quote(str(c)) for c in cmd), flush=True)
	res = subprocess.run(
		[str(c) for c in cmd], cwd=cwd, input=input_bytes,
		stdout=subprocess.PIPE if capture else None,
	)
	if check and res.returncode != 0:
		raise ReleaseError(f"Command failed with exit code {res.returncode}: {cmd[0]}")
	return res.stdout if capture else res.returncode


def confirm(question, assume_yes=False):
	"""Asks a yes/no question, default is 'no'. Returns True for yes."""
	if assume_yes:
		print(f"{question} [y/N] y (--yes)")
		return True
	try:
		answer = input(f"{question} [y/N] ")
	except EOFError:
		return False
	return answer.strip().lower() in ("y", "yes", "j", "ja")
