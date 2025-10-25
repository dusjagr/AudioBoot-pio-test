#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <filesystem>
#include <iostream>

#include "Adafruit_NeoPixel.h"

// Arduino shim: declare functions/macros before including visuals.h so calls resolve
uint32_t millis();
void delay(uint16_t ms);
void randomSeed(unsigned long seed);

// Provide our own random(...) via macro to avoid clashing with libc random()
#undef random
inline long rnd(long howbig) {
  if (howbig <= 0) return 0;
  return std::rand() % howbig;
}
inline long rnd(long howsmall, long howbig) {
  if (howsmall >= howbig) return howsmall;
  return howsmall + rnd(howbig - howsmall);
}
#define random(...) rnd(__VA_ARGS__)

#ifndef min
#define min(a,b) (( (a) < (b) ) ? (a) : (b))
#endif

#include "../include/matrix_helpers.h"
#include "../include/visuals.h"

// ---- Arduino-like timing ----
static auto g_start = std::chrono::steady_clock::now();
uint32_t millis() {
  auto now = std::chrono::steady_clock::now();
  return (uint32_t) std::chrono::duration_cast<std::chrono::milliseconds>(now - g_start).count();
}
void delay(uint16_t ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ---- Arduino randomSeed shim ----
void randomSeed(unsigned long seed);

// ---- Global pixels as expected by helpers/visuals ----
Adafruit_NeoPixel pixels;

// ---- Wheel function used by visuals ----
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return Adafruit_NeoPixel::Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return Adafruit_NeoPixel::Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return Adafruit_NeoPixel::Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// ---- Unused on emulator ----
void playSound(uint16_t /*freq*/, uint16_t /*dur*/) {}

// Implement randomSeed after including standard headers
void randomSeed(unsigned long seed) { std::srand((unsigned)seed); }

// ---- Frame writer (PPM) ----
struct RGB { uint8_t r,g,b; };
static inline RGB unpack(uint32_t c) { return RGB{ uint8_t((c>>16)&0xFF), uint8_t((c>>8)&0xFF), uint8_t(c&0xFF)}; }

// cellW/cellH allow non-square pixels; use 5:4 ratio by default (e.g., 30x24)
void write_ppm(const std::string& path, const std::vector<uint32_t>& buf, int mw, int mh, int cellW=30, int cellH=24) {
  int W = mw * cellW;
  int H = mh * cellH;
  std::vector<uint8_t> img(W*H*3, 0);
  // fill background dark
  for (size_t i=0;i<img.size();++i) img[i]=10;
  // draw cells
  for (int y=0;y<mh;++y) {
    for (int x=0;x<mw;++x) {
      int idx = matrixIndex((uint8_t)x,(uint8_t)y);
      if (idx<0) continue;
      RGB c = unpack(buf[(size_t)idx]);
      for (int dy=1; dy<cellH-1; ++dy) {
        for (int dx=1; dx<cellW-1; ++dx) {
          int px = x*cellW + dx;
          int py = y*cellH + dy;
          size_t off = (size_t)(py*W + px)*3;
          img[off+0]=c.r; img[off+1]=c.g; img[off+2]=c.b;
        }
      }
    }
  }
  // write binary PPM (P6)
  std::filesystem::create_directories(std::filesystem::path(path).parent_path());
  FILE* f = std::fopen(path.c_str(), "wb");
  if (!f) return;
  std::fprintf(f, "P6\n%d %d\n255\n", W, H);
  std::fwrite(img.data(), 1, img.size(), f);
  std::fclose(f);
}

int main(int argc, char** argv) {
  // configure pixels buffer
  pixels.resize(MATRIX_W * MATRIX_H);
  pixels.setBrightness(255);

  // output directory and frames
  std::string outdir = "emu_out/frames";
  int frames_limit = 120; // cap frames saved; visual runtime also limits output
  int runtime_ms = 4000;  // default visual runtime
  if (const char* s = std::getenv("EMU_OUT")) outdir = s;
  if (const char* s = std::getenv("EMU_FRAMES")) frames_limit = std::max(1, std::atoi(s));
  if (const char* s = std::getenv("EMU_RUNTIME_MS")) runtime_ms = std::max(100, std::atoi(s));

  int frame_no = 0;
  pixels.setShowCallback([&](const std::vector<uint32_t>& data){
    if (frame_no >= frames_limit) return; // cap frames
    char name[256];
    std::snprintf(name, sizeof(name), "%s/frame_%04d.ppm", outdir.c_str(), frame_no++);
    // Render with 5:4 pixel aspect (e.g., 30x24)
    write_ppm(name, data, MATRIX_W, MATRIX_H, 30, 24);
  });

  // Seed RNG deterministically for repeatable previews
  std::srand(1);

  // Dispatch by VISUAL_ID similar to firmware loop
#ifndef VISUAL_ID
#define VISUAL_ID 18
#endif

  // Determine which visual to run: compile-time VISUAL_ID or runtime override via env
  int vid = VISUAL_ID;
  if (const char* s = std::getenv("EMU_VID")) vid = std::atoi(s);
  if (const char* s = std::getenv("EMU_VISUAL_ID")) vid = std::atoi(s);

  auto run_visual = [&](){
    switch (vid) {
      case 0:  matrixLarsonScanner((uint16_t)runtime_ms, 80); break;
      case 1:  matrixBouncingDot((uint16_t)runtime_ms, 50); break;
      case 2:  matrixRain((uint16_t)runtime_ms, 60); break;
      case 3:  matrixTwinkle((uint16_t)runtime_ms, 50); break;
      case 4:  matrixWipe((uint16_t)runtime_ms, 15); break;
      case 5:  matrixSpinner((uint16_t)runtime_ms, 40); break;
      case 6:  matrixFire((uint16_t)runtime_ms, 120, 80, 60); break;
      case 7:  matrixRainbowZoom((uint16_t)runtime_ms, 60); break;
      case 8:  matrixPinkSpiral((uint16_t)runtime_ms, 50); break;
      case 9:  matrixPong((uint16_t)runtime_ms, 50); break;
      case 10: matrixDnBDancer((uint16_t)runtime_ms, 220); break;
      case 11: matrixKanjiScroll((uint16_t)runtime_ms, 80); break;
      case 12: matrixExplosion((uint16_t)runtime_ms, 60); break;
      case 13: matrixShoggoth((uint16_t)runtime_ms, 60); break;
      case 14: matrixRainbowWaves((uint16_t)runtime_ms, 60); break;
      case 15: matrixTetris((uint16_t)runtime_ms, 120); break;
      case 16: matrixFiveEightSeam((uint16_t)runtime_ms, 240, 4); break;
      case 17: matrixCoteAzur((uint16_t)runtime_ms, 60); break;
      case 18: matrixSunsetPickleSun((uint16_t)runtime_ms, 70); break;
      case 19: matrixLightning((uint16_t)runtime_ms); break;
      case 20: matrixFlagsShow((uint16_t)runtime_ms, 700); break;
      case 21: matrixFlagsShowFade((uint16_t)runtime_ms, 800, 300, 6); break;
      case 22: matrixDigitalRain((uint16_t)runtime_ms, 90); break;
      case 23: matrixWaterfall((uint16_t)runtime_ms, 90); break;
      case 24: matrixLarsonScannerDual((uint16_t)runtime_ms, 90); break;
      case 25: matrixNightStreet2000((uint16_t)runtime_ms, 80); break;
      case 26: matrixSwitchToggle((uint16_t)runtime_ms, 120); break;
      default: matrixSunsetPickleSun((uint16_t)runtime_ms, 70); break;
    }
  };

  run_visual();

  std::cout << "Frames written: " << frame_no << " into " << outdir << "\n";
  std::cout << "Convert to GIF with: python3 scripts/gen_visual_gif.py --in " << outdir << " --out emu_out/preview.gif" << std::endl;
  return 0;
}
