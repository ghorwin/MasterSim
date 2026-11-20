#!/usr/bin/env python3
"""Builds the widget gallery for every style variant and renders the screenshots.

Usage: render_shots.py <variants-dir> <build-dir> <output-dir>

For each variant (subdirectory of <variants-dir>) plus Qt's Fusion style, the
gallery is rendered in a light and a dark palette into
<output-dir>/shots/<variant>/<light|dark>/<section>.png.

Rendering uses the offscreen platform plugin with 2x scaling, so no display is
needed and all screenshots use the same font and resolution.
"""
import argparse
import os
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("variants_dir", type=Path)
    ap.add_argument("build_dir", type=Path)
    ap.add_argument("output_dir", type=Path)
    args = ap.parse_args()

    variants = sorted(d.name for d in args.variants_dir.iterdir() if (d / "phantomstyle.cpp").exists())
    variants.append("fusion")

    subprocess.run(["cmake", "-S", str(HERE), "-B", str(args.build_dir),
                    "-DCMAKE_BUILD_TYPE=Release",
                    f"-DVARIANTS_DIR={args.variants_dir.resolve()}"], check=True)
    subprocess.run(["cmake", "--build", str(args.build_dir), "-j", str(os.cpu_count() or 4)], check=True)

    env = dict(os.environ, QT_QPA_PLATFORM="offscreen", QT_SCALE_FACTOR="2")
    for v in variants:
        exe = args.build_dir / f"gallery_{v}"
        for palette in ("light", "dark"):
            out = args.output_dir / "shots" / v / palette
            print(f"rendering {v} ({palette})")
            subprocess.run([str(exe), palette, str(out)], env=env, check=True, timeout=60)


if __name__ == "__main__":
    main()
