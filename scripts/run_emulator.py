#!/usr/bin/env python3
"""
Build + run the native emulator and generate an animated GIF in one command.

Usage examples:
  python3 scripts/run_emulator.py                      # uses VISUAL_ID from platformio.ini, writes emu_out/preview.gif
  python3 scripts/run_emulator.py --vid 7 --frames 90  # override visual and frame count
  python3 scripts/run_emulator.py --out emu_out/my.gif --fps 12

Requires:
  - PlatformIO CLI (platformio)
  - ImageMagick `convert` or Python Pillow for GIF
"""
import argparse
import os
import shutil
import subprocess
import tempfile
from pathlib import Path
from typing import Optional

ROOT = Path(__file__).resolve().parents[1]
PioExe = shutil.which("platformio") or "platformio"
BUILD_PROG = ROOT / ".pio" / "build" / "native" / "program"


def run(cmd, cwd=None, env=None):
    print("[emu] $ "+" ".join(cmd))
    return subprocess.run(cmd, cwd=cwd, env=env, text=True)


def patch_platformio_ini(vid: Optional[int]) -> Path:
    """Create a temporary platformio.ini that optionally appends -DVISUAL_ID to [env:native]."""
    base = (ROOT / "platformio.ini").read_text()
    if vid is None:
        # No patching needed; just use original file
        tf = tempfile.NamedTemporaryFile("w", delete=False, suffix=".ini")
        tf.write(base)
        tf.close()
        return Path(tf.name)

    lines = base.splitlines()
    out = []
    in_native = False
    injected = False
    for line in lines:
        s = line.strip()
        if s.startswith("[") and s.endswith("]"):
            # leaving native section: if not injected, append a build_flags line
            if in_native and not injected:
                out.append(f"build_flags = -std=c++17 -I emu -I include -DMATRIX_W=5 -DMATRIX_H=4 -DVISUAL_ID={vid}")
                injected = True
            in_native = (s.lower() == "[env:native]")
            out.append(line)
            continue
        if in_native and s.lower().startswith("build_flags"):
            # augment existing flags with -DVISUAL_ID
            try:
                key, val = line.split('=', 1)
                val = val.rstrip() + f" -DVISUAL_ID={vid}"
                out.append(f"{key}={val}")
            except ValueError:
                out.append(line)
            injected = True
            continue
        out.append(line)
    # end of file, still in native
    if in_native and not injected:
        out.append(f"build_flags = -std=c++17 -I emu -I include -DMATRIX_W=5 -DMATRIX_H=4 -DVISUAL_ID={vid}")
    tf = tempfile.NamedTemporaryFile("w", delete=False, suffix=".ini")
    tf.write("\n".join(out) + "\n")
    tf.close()
    return Path(tf.name)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--vid", type=int, default=None, help="Override VISUAL_ID for emulator")
    ap.add_argument("--frames", type=int, default=120, help="Max frames to render")
    ap.add_argument("--fps", type=int, default=10, help="GIF frames per second")
    ap.add_argument("--out", default=str(ROOT / "emu_out" / "preview.gif"), help="Output GIF path")
    ap.add_argument("--frames-dir", default=str(ROOT / "emu_out" / "frames"), help="Where to write PPM frames")
    ap.add_argument("--cleanup", action="store_true", help="Delete input frames after GIF is written")
    args = ap.parse_args()

    # 1) Build native with optional VISUAL_ID override via temp config
    tconf = patch_platformio_ini(args.vid)
    try:
        rc = run([PioExe, "run", "-e", "native", "--project-conf", str(tconf)], cwd=str(ROOT)).returncode
        if rc != 0:
            print("[emu] Build failed")
            return rc
    finally:
        try:
            os.unlink(tconf)
        except Exception:
            pass

    # 2) Run emulator to write frames
    env = os.environ.copy()
    env["EMU_OUT"] = args.frames_dir
    env["EMU_FRAMES"] = str(args.frames)
    if args.vid is not None:
        env["EMU_VID"] = str(args.vid)
    rc = run([str(BUILD_PROG)], cwd=str(ROOT), env=env).returncode
    if rc != 0:
        print("[emu] Emulator run failed")
        return rc

    # 3) Make GIF
    outgif = Path(args.out)
    outgif.parent.mkdir(parents=True, exist_ok=True)
    gif_cmd = [
        "python3", str(ROOT / "scripts" / "gen_visual_gif.py"),
        "--in", args.frames_dir,
        "--out", str(outgif),
        "--fps", str(args.fps),
    ]
    if args.cleanup:
        gif_cmd.append("--cleanup")
    rc = run(gif_cmd, cwd=str(ROOT)).returncode
    if rc == 0:
        print("[emu] GIF ready:", outgif)
    return rc


if __name__ == "__main__":
    raise SystemExit(main())
