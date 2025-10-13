# AudioBoot PlatformIO Test (ATtiny85)

This project demonstrates an ATtiny85 PlatformIO setup with a working audio-boot upload workflow similar to the 8Bit Mixtape. It converts the compiled firmware (`.hex`) to an audio waveform (`.wav`) and optionally auto-plays it through your computer’s audio output to program the device via an audio bootloader.

Repository: https://github.com/dusjagr/AudioBoot-pio-test

## Features

- ATtiny85 using Arduino framework @ 16 MHz
- NeoPixel animation helpers for a 5x4 matrix mapped to a 20-pixel strip
- OneWire temperature example scaffolding (DS18x20 on `PB4`)
- Post-build script to convert HEX → WAV via `tools/hex2wav` and auto-play via `aplay`
- Custom `upload_command` so `platformio run -t upload` will generate and play the audio

## Hardware Setup (reference)

- MCU: ATtiny85 @ 16 MHz
- NeoPixel strip/matrix:
  - Data pin: `0` (Arduino numbering)
  - LED count: `20`
- Speaker / piezo: `1`
- OneWire temperature sensor (e.g., DS18B20): `PB4` (with 4.7k pull-up to VCC)

See `src/AetzLampliu_mini.ino` for pin `#define`s and example animations.

## Requirements

- PlatformIO Core (CLI) or PlatformIO in VS Code
- Linux with ALSA `aplay` available (used to play the generated WAV)
- The repository includes platform-specific `hex2wav` tools under `tools/hex2wav/`.
  - Linux binary is used by default via `tools/hex2wav/linux/hex2wav64_bin`

If you are on macOS or Windows, you can adjust the commands in `platformio.ini` to point to the platform-specific tool.

## Quick Start

1) Build

```bash
platformio run
```

2) Generate and play the audio upload waveform automatically

```bash
platformio run -t upload
```

This runs the custom `upload_command` defined in `platformio.ini`, which:
- Builds the firmware
- Converts `.pio/build/<env>/<progname>.hex` → `.wav`
- Plays the `.wav` via `aplay`

3) Alternatively, just build and let the post-action auto-play (if enabled):

```bash
platformio run
```

The post script `scripts/hex2wav_post.py` is hooked via `extra_scripts` and may auto-play depending on config.

## Configuration (`platformio.ini`)

Key options you can tweak:

- `board_build.f_cpu = 16000000L` and `-DF_CPU=16000000L`
- `lib_deps = OneWire, adafruit/Adafruit NeoPixel`
- `extra_scripts = scripts/hex2wav_post.py`
- `hex2wav_cmd = tools/hex2wav/linux/hex2wav64_bin {hex} {wav}`
- `hex2wav_auto_play = yes`
- `hex2wav_player_cmd = aplay -q {wav}`
- `upload_protocol = custom`
- `upload_command = tools/hex2wav/linux/hex2wav64_bin $SOURCE .pio/build/${PIOENV}/${PROGNAME}.wav && aplay -q .pio/build/${PIOENV}/${PROGNAME}.wav`

If you don’t want automatic playback after build, set:

```ini
hex2wav_auto_play = no
```

Or change `hex2wav_player_cmd` to another player.

## Project Structure

- `src/AetzLampliu_mini.ino` — Main sketch with NeoPixel matrix helpers and animations
- `scripts/hex2wav_post.py` — Post-build helper to convert HEX → WAV and (optionally) play it
- `tools/hex2wav/` — Platform-specific tools used for the conversion
- `platformio.ini` — PlatformIO environment configuration and custom upload

## Troubleshooting

- No sound during upload: ensure your system volume is up and `aplay` is installed/working.
- Device not responding to audio: double-check audio cable, levels, and that the bootloader expects this encoding.
- NeoPixels not lighting: verify power supply, ground, and that the data pin in code matches your wiring.
- OneWire sensor absent: ensure a 4.7k pull-up on the data line and correct wiring to `PB4`.

