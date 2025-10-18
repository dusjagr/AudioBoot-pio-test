#!/usr/bin/env python3
"""
Convert a directory of PPM frames (as written by the emulator) into an animated GIF.

Usage:
  python3 scripts/gen_visual_gif.py --in emu_out/frames --out emu_out/preview.gif --fps 10

Requires ImageMagick `convert` OR, optionally, Pillow if installed.
"""
import argparse
import os
import shutil
import subprocess
from pathlib import Path

def run(cmd):
    print("[gif] $ "+" ".join(cmd))
    return subprocess.call(cmd)

def make_with_imagemagick(srcs, out_path, fps):
    delay_cs = int(100 / fps)  # delay in 1/100s units
    cmd = ["convert", "-delay", str(delay_cs), "-loop", "0"] + srcs + [str(out_path)]
    return run(cmd) == 0


def make_with_pillow(srcs, out_path, fps):
    try:
        from PIL import Image
    except Exception:
        return False
    frames = [Image.open(s) for s in srcs]
    first, rest = frames[0], frames[1:]
    duration_ms = int(1000 / fps)
    first.save(out_path, save_all=True, append_images=rest, duration=duration_ms, loop=0)
    return True


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in", dest="indir", required=True, help="Directory with frame_XXXX.ppm")
    ap.add_argument("--out", dest="outfile", required=True, help="Output GIF path")
    ap.add_argument("--fps", type=int, default=10)
    ap.add_argument("--cleanup", action="store_true", help="Delete input frames after GIF is written")
    args = ap.parse_args()

    indir = Path(args.indir)
    out = Path(args.outfile)
    out.parent.mkdir(parents=True, exist_ok=True)

    frames = sorted(str(p) for p in indir.glob("frame_*.ppm"))
    if not frames:
        print("[gif] No frames found in", indir)
        return 1

    # Prefer ImageMagick if available
    if shutil.which("convert"):
        ok = make_with_imagemagick(frames, out, args.fps)
        if ok:
            print("[gif] Wrote", out)
            if args.cleanup:
                for p in frames:
                    try:
                        Path(p).unlink()
                    except Exception:
                        pass
            return 0
        print("[gif] ImageMagick convert failed, trying Pillow...")

    ok = make_with_pillow(frames, out, args.fps)
    if ok:
        print("[gif] Wrote", out)
        if args.cleanup:
            for p in frames:
                try:
                    Path(p).unlink()
                except Exception:
                    pass
        return 0

    print("[gif] Could not create GIF. Install ImageMagick or Pillow (pip install pillow).")
    return 2

if __name__ == "__main__":
    raise SystemExit(main())
