# Visual Emulator + GIF — Quick Guide

This guide shows only how to preview visuals on your laptop and create animated GIFs. It reuses the same C/C++ visuals from the firmware.

## 1) Build the native emulator (one‑time)

```
platformio run -e native
```

This produces the emulator program at:

- `.pio/build/native/program`

## 2) Run the emulator to render frames

By default, frames are written as PPM images to `emu_out/frames/`.

- Simple run (uses `VISUAL_ID` from `platformio.ini`):
```
.pio/build/native/program
```

- Control runtime and max saved frames via environment variables:
```
EMU_RUNTIME_MS=12000 EMU_FRAMES=240 .pio/build/native/program
```

Definitions:
- `EMU_RUNTIME_MS`: how long the visual runs (milliseconds). Default 4000.
- `EMU_FRAMES`: cap of frames saved. Default 120.

## 3) Convert frames to an animated GIF

Use the provided helper (requires ImageMagick `convert` or Python Pillow):
```
python3 scripts/gen_visual_gif.py --in emu_out/frames --out emu_out/preview.gif --fps 12
```

Output:
- `emu_out/preview.gif`

## 4) One‑command workflow (build + run + GIF)

Use the convenience script:
```
python3 scripts/run_emulator.py
```

Options:
```
# Pick a visual ID, limit frames, change GIF FPS and output path
python3 scripts/run_emulator.py --vid 7 --frames 200 --fps 12 --out emu_out/visual-07.gif

# You can also extend runtime with an env var
EMU_RUNTIME_MS=12000 python3 scripts/run_emulator.py --vid 7 --frames 240 --fps 12
```

## 5) Change which visual runs (VID)

Pick one of the following methods:

- Temporary (recommended): pass on the command line when using the helper script
```
python3 scripts/run_emulator.py --vid 18
```

- Permanent (for repeated runs): set it in `platformio.ini` under `[env:native]`:
```
[env:native]
…
build_flags = -std=c++17 -I emu -I include -DMATRIX_W=5 -DMATRIX_H=4 -DVISUAL_ID=18
```

That’s it — build, run, and make your GIF!
