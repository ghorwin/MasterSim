#!/usr/bin/env python3
"""Collects the source files of all Phantom style variants that are compared.

Usage: fetch_variants.py <variants-dir> [--vicus <path-to-SIM-VICUS-checkout>] [--original <src-dir>]

Creates one subdirectory per variant in <variants-dir>, each containing
phantomstyle.{h,cpp}, phantomcolor.{h,cpp} and phantomtweak.h:

  original      - our vendored copy in externals/phantom-style/src (Qt6-patched)
  julienmaille  - github.com/JulienMaille/phantomstyle (pinned commit, see FORKS)
  cristianadam  - github.com/cristianadam/phantomstyle (pinned commit)
  jcelerier     - github.com/jcelerier/phantomstyle (pinned commit)
  vicus         - SIM-VICUS externals/phantom/src, taken from a local checkout
                  (--vicus). The VICUS repository is private, so without --vicus
                  this variant is skipped.

The fork commits are pinned so that the comparison can be reproduced; update the
SHAs below to compare newer versions.
"""
import argparse
import shutil
import sys
import urllib.request
from pathlib import Path

FILES = ["phantomstyle.cpp", "phantomstyle.h", "phantomcolor.cpp", "phantomcolor.h", "phantomtweak.h"]

# variant name -> (GitHub repository, commit SHA, source directory in repository)
FORKS = {
    "julienmaille": ("JulienMaille/phantomstyle", "356703a0fbc6f2e9f1dd9d128c4157b1e8957895", "src/phantom"),  # 2026-09-16
    "cristianadam": ("cristianadam/phantomstyle", "c654231e4468c76e50d34e4c1d50294d89a2a348", "src/phantom"),  # 2025-12-29
    "jcelerier":    ("jcelerier/phantomstyle",    "e49cf4e47fcc3a72ee34cd85972b62fde18795ef", "src/phantom"),  # 2026-10-06
}
# source directory in a SIM-VICUS checkout; compared state: commit 266c8cef (2026-11-14)
VICUS_SUBDIR = "externals/phantom/src"

# our vendored copy: <repo>/externals/phantom-style/src (this script lives in externals/phantom-style/doc/<dir>/)
ORIGINAL_SRC = Path(__file__).resolve().parents[2] / "src"


def download(repo, sha, subdir, target: Path):
    target.mkdir(parents=True, exist_ok=True)
    for f in FILES:
        url = f"https://raw.githubusercontent.com/{repo}/{sha}/{subdir}/{f}"
        print(f"  {url}")
        with urllib.request.urlopen(url, timeout=60) as r:
            (target / f).write_bytes(r.read())


def copy_local(src: Path, target: Path):
    target.mkdir(parents=True, exist_ok=True)
    for f in FILES:
        shutil.copy2(src / f, target / f)


def patch_julienmaille(target: Path):
    """JulienMaille's fork uses PM_ButtonIconLabelMargin, which only exists in the
    author's patched Qt. Remove that case so it builds against standard Qt."""
    p = target / "phantomstyle.cpp"
    s = p.read_text(encoding="utf-8")
    snippet = ("  case PM_ButtonIconLabelMargin:  // this PixelMetric enum comes from our own Qt5 patch\n"
               "    val = -15;\n"
               "    break;\n")
    if snippet in s:
        p.write_text(s.replace(snippet, ""), encoding="utf-8")
        print("  patched: removed PM_ButtonIconLabelMargin case")
    elif "PM_ButtonIconLabelMargin" in s:
        sys.exit("JulienMaille: PM_ButtonIconLabelMargin found, but patch snippet does not match - adjust patch_julienmaille()")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("variants_dir", type=Path)
    ap.add_argument("--vicus", type=Path, help="path to a local SIM-VICUS checkout (if omitted, the VICUS variant is skipped)")
    ap.add_argument("--original", type=Path, default=ORIGINAL_SRC,
                    help=f"source directory of the original variant (default: {ORIGINAL_SRC})")
    args = ap.parse_args()
    out = args.variants_dir

    print("original (vendored copy)")
    copy_local(args.original, out / "original")

    for name, (repo, sha, subdir) in FORKS.items():
        print(name)
        download(repo, sha, subdir, out / name)
    patch_julienmaille(out / "julienmaille")

    if args.vicus:
        print("vicus")
        copy_local(args.vicus / VICUS_SUBDIR, out / "vicus")
    else:
        print("vicus: skipped (no --vicus path given)")

    print(f"Variants written to {out}")


if __name__ == "__main__":
    main()
