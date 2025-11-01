#!/usr/bin/env python3
"""
Batch-generate animated GIF previews for all visuals using the native emulator.

This calls scripts/run_emulator.py per visual with --vid, writes GIFs to emu_out/gifs/.

Usage:
  python3 scripts/gen_visual_gifs.py                 # default settings
  python3 scripts/gen_visual_gifs.py --fps 12 --frames 200 --runtime 12000

Requirements:
  - PlatformIO CLI (for native build used by run_emulator.py)
  - ImageMagick `convert` or Python Pillow
"""
import argparse
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTDIR = ROOT / "wav" / "gifs"
RUNNER = ROOT / "scripts" / "run_emulator.py"

# List of visuals (id, slug)
VISUALS = [
    (0,  "larson"),
    (1,  "bouncing-dot"),
    (2,  "rain"),
    (3,  "twinkle"),
    (4,  "wipe"),
    (5,  "spinner"),
    (6,  "fire"),
    (7,  "rainbow-zoom"),
    (8,  "pink-spiral"),
    (9,  "pong"),
    (10, "dnb-dancer"),
    (11, "kanji-scroll"),
    (12, "explosion"),
    (13, "shoggoth"),
    (14, "rainbow-waves"),
    (15, "tetris"),
    (16, "five-eight-seam"),
    (17, "cote-azur"),
    (18, "sunset-pickle-sun"),
    (19, "lightning"),
    (20, "flags"),
    (21, "flags-fade"),
    (22, "digital-rain"),
    (23, "waterfall"),
    (24, "larson-dual"),
    (25, "night-street-2000"),
    (26, "switch-toggle"),
    (27, "blue-screen"),
    (28, "snow-white"),
    (29, "big-bad-wolf"),
    (30, "trickster-plasma"),
]


def run(cmd):
    print("[gifs] $ "+" ".join(cmd))
    return subprocess.run(cmd)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--fps", type=int, default=12, help="GIF frames per second")
    ap.add_argument("--frames", type=int, default=200, help="Max frames per visual")
    ap.add_argument("--runtime", type=int, default=8000, help="EMU_RUNTIME_MS per visual")
    ap.add_argument("--only", type=str, default="", help="Comma-separated list of IDs to include (e.g. 0,6,21)")
    args = ap.parse_args()

    OUTDIR.mkdir(parents=True, exist_ok=True)

    include = None
    if args.only.strip():
        include = {int(x.strip()) for x in args.only.split(',') if x.strip().isdigit()}

    failures = []
    for vid, slug in VISUALS:
        if include is not None and vid not in include:
            continue
        outgif = OUTDIR / f"{vid:02d}_{slug}.gif"
        env = {
            **dict(),
        }
        # Build + run + gif in one go; we set runtime via environment and cleanup frames after
        cmd = [
            "python3", str(RUNNER),
            "--vid", str(vid),
            "--frames", str(args.frames),
            "--fps", str(args.fps),
            "--out", str(outgif),
            "--cleanup",
        ]
        # Pass EMU_RUNTIME_MS via env to the runner (which forwards to program)
        full_env = dict(**{k: v for k, v in env.items()})
        # Let the child inherit, but override runtime
        import os
        full_env.update(os.environ)
        full_env["EMU_RUNTIME_MS"] = str(args.runtime)
        print(f"[gifs] Rendering VID={vid} ({slug}) -> {outgif}")
        res = subprocess.run(cmd, cwd=str(ROOT), env=full_env)
        if res.returncode != 0:
            failures.append((vid, slug, res.returncode))
    if failures:
        print("\n[gifs] Completed with errors:")
        for vid, slug, rc in failures:
            print(f"  - {vid}:{slug} -> exit {rc}")
        return 1
    print("\n[gifs] All GIFs generated in:", OUTDIR)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
