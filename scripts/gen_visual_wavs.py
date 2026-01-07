#!/usr/bin/env python3
"""
Generate individual WAV files for each visual by building the firmware multiple times
with different VISUAL_ID values. Requires PlatformIO CLI in PATH.

Usage:
  python3 scripts/gen_visual_wavs.py

Outputs are saved to the project's wav/ directory as:
  wav/visual-<id>-<slug>.wav

This script temporarily disables auto-playback to speed up batch generation.
"""
import os
import shutil
import subprocess
import sys
from pathlib import Path
import tempfile

PROJECT_ROOT = Path(__file__).resolve().parents[1]
BUILD_DIR = PROJECT_ROOT / ".pio" / "build" / "attiny85"
WAV_DIR = PROJECT_ROOT / "wav"

# List of visuals to build: (id, slug)
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
    (31, "pink-slime-sprouts"),
    (32, "milky-sea"),
    (33, "rainbow-half-donut"),
    (34, "rainbow-planet"),
    (35, "starry-sky"),
    (36, "fog-noise"),
    (37, "rainbow-tiled-noise"),
    (38, "universe-creation"),
    (39, "spear"),
    (40, "christmas-tree-aura"),
    (41, "ccc-rocket"),
    (42, "beach-wave"),
    (43, "antifa-flag"),
    (44, "trickster-plasma"),
    (45, "chaotic-pink"),
]

# Brightness levels to generate
BRIGHTNESS_LEVELS = [
    ("low", 20),
    ("med", 80),
    ("high", 255),
]


def run(cmd, cwd=None):
    print("[genwavs] $ " + " ".join(cmd))
    proc = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    print(proc.stdout)
    return proc.returncode


def ensure_dirs():
    WAV_DIR.mkdir(parents=True, exist_ok=True)
    BUILD_DIR.mkdir(parents=True, exist_ok=True)


def build_one(vid: int, slug: str, brightness_val: int) -> Path:
    """Build with VISUAL_ID=vid and GLOBAL_BRIGHTNESS=brightness_val, return path to produced wav."""
    # Compose a temporary platformio.ini by patching the existing env section
    base_ini = (PROJECT_ROOT / "platformio.ini").read_text()
    lines = base_ini.splitlines()
    out_lines = []
    in_env = False
    inserted_custom_autoplay = False
    modified_build_flags = False
    for i, line in enumerate(lines):
        stripped = line.strip()
        # Track section boundaries
        if stripped.startswith('[') and stripped.endswith(']'):
            # Leaving env block? If we were inside and didn't insert overrides, add them before leaving
            if in_env and not inserted_custom_autoplay:
                out_lines.append(f"custom_hex2wav_auto_play = no")
                inserted_custom_autoplay = True
            in_env = (stripped.lower() == "[env:attiny85]")
            out_lines.append(line)
            continue

        if in_env:
            # Force custom_hex2wav_auto_play to no
            if stripped.lower().startswith("custom_hex2wav_auto_play"):
                out_lines.append("custom_hex2wav_auto_play = no")
                inserted_custom_autoplay = True
                continue
            # Append -DVISUAL_ID and -DGLOBAL_BRIGHTNESS to build_flags (preserve existing flags)
            if stripped.lower().startswith("build_flags"):
                # Keep everything after '=' and append our define
                try:
                    key, val = line.split('=', 1)
                    val = val.rstrip() + f" -DVISUAL_ID={vid} -DGLOBAL_BRIGHTNESS={brightness_val}"
                    out_lines.append(f"{key}={val}")
                except ValueError:
                    out_lines.append(line)
                modified_build_flags = True
                continue

        out_lines.append(line)

    # If env section existed but we didn't see a build_flags line, insert one at the end of the env block
    if "[env:attiny85]" in base_ini and not modified_build_flags:
        patched = []
        in_env = False
        for line in out_lines:
            stripped = line.strip()
            if stripped.startswith('[') and stripped.endswith(']'):
                # Before starting a new section, if we're leaving env and didn't add build_flags yet, add it
                if in_env:
                    patched.append(f"build_flags = -DVISUAL_ID={vid} -DGLOBAL_BRIGHTNESS={brightness_val}")
                    if not inserted_custom_autoplay:
                        patched.append("custom_hex2wav_auto_play = no")
                in_env = (stripped.lower() == "[env:attiny85]")
                patched.append(line)
            else:
                patched.append(line)
        # If file ended while still in env
        if in_env:
            patched.append(f"build_flags = -DVISUAL_ID={vid} -DGLOBAL_BRIGHTNESS={brightness_val}")
            if not inserted_custom_autoplay:
                patched.append("custom_hex2wav_auto_play = no")
        out_text = "\n".join(patched) + "\n"
    else:
        out_text = "\n".join(out_lines) + "\n"

    with tempfile.TemporaryDirectory() as tdir:
        tconf = Path(tdir) / "platformio.ini"
        tconf.write_text(out_text)
        cmd = [
            "platformio", "run",
            "-e", "attiny85",
            "--project-dir", str(PROJECT_ROOT),
            "--project-conf", str(tconf),
        ]
        rc = run(cmd, cwd=str(PROJECT_ROOT))
        if rc != 0:
            raise RuntimeError(f"Build failed for VISUAL_ID={vid} Brightness={brightness_val}")
    # Convert HEX -> WAV directly using local tools
    hex_path = BUILD_DIR / "firmware.hex"
    if not hex_path.exists():
        raise FileNotFoundError(f"Expected HEX not found: {hex_path}")

    tools_dir = PROJECT_ROOT / "tools" / "hex2wav"
    linux_bin = tools_dir / "linux" / "hex2wav64_bin"
    jar_path = tools_dir / "hex2wav.jar"
    wav_out = BUILD_DIR / "firmware.wav"

    if linux_bin.exists():
        rc = run([str(linux_bin), str(hex_path), str(wav_out)], cwd=str(PROJECT_ROOT))
        if rc != 0:
            raise RuntimeError("hex2wav64_bin failed")
    elif jar_path.exists():
        rc = run(["java", "-jar", str(jar_path), "-i", str(hex_path), "-o", str(wav_out)], cwd=str(PROJECT_ROOT))
        if rc != 0:
            raise RuntimeError("java hex2wav.jar failed")
    else:
        raise FileNotFoundError("No hex2wav tool found under tools/hex2wav/")

    if not wav_out.exists():
        raise FileNotFoundError(f"Expected WAV not found: {wav_out}")
    return wav_out


def main():
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", type=str, default="", help="Comma-separated list of IDs to build (e.g. 6,18,22)")
    args = ap.parse_args()

    ensure_dirs()
    failures = []

    include = None
    if args.only.strip():
        include = {int(x.strip()) for x in args.only.split(',') if x.strip().isdigit()}

    for vid, slug in VISUALS:
        if include is not None and vid not in include:
            continue

        # Build for each brightness level
        for b_name, b_val in BRIGHTNESS_LEVELS:
            try:
                print(f"[genwavs] Building ID={vid} ({slug}) Brightness={b_name} ({b_val})...")
                built_wav = build_one(vid, slug, b_val)
                out_name = f"visual-{vid:02d}-{slug}-{b_name}.wav"
                out_path = WAV_DIR / out_name
                shutil.copy2(built_wav, out_path)
                print(f"[genwavs] Saved: {out_path}")

                # If high brightness, also save as default name for backward compatibility
                if b_name == "high":
                    default_name = f"visual-{vid:02d}-{slug}.wav"
                    default_path = WAV_DIR / default_name
                    shutil.copy2(built_wav, default_path)
                    print(f"[genwavs] Saved default: {default_path}")

            except Exception as e:
                print(f"[genwavs] ERROR for {vid}:{slug} -> {e}")
                failures.append((vid, slug, str(e)))
    if failures:
        print("\n[genwavs] Completed with errors:")
        for vid, slug, err in failures:
            print(f"  - {vid}:{slug} -> {err}")
        sys.exit(1)
    else:
        print("\n[genwavs] All visuals built successfully.")


if __name__ == "__main__":
    main()
