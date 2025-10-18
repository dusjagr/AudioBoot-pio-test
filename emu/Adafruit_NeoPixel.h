#pragma once
#include <cstdint>
#include <vector>
#include <functional>

using byte = uint8_t;

// Minimal stub of Adafruit_NeoPixel for host-side emulation
class Adafruit_NeoPixel {
public:
  Adafruit_NeoPixel(uint16_t n, uint8_t /*pin*/, uint8_t /*type*/) : count(n), pixels_(n, 0), brightness_(255) {}
  Adafruit_NeoPixel() : count(0), brightness_(255) {}

  void begin() {}
  void setBrightness(uint8_t b) { brightness_ = b; }
  uint16_t numPixels() const { return count; }

  static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
    return (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
  }

  void setPixelColor(uint16_t i, uint32_t c) {
    if (i < pixels_.size()) pixels_[i] = applyBrightness(c);
  }
  void setPixelColor(uint16_t i, uint8_t r, uint8_t g, uint8_t b) {
    setPixelColor(i, Color(r,g,b));
  }
  uint32_t getPixelColor(uint16_t i) const {
    if (i < pixels_.size()) return pixels_[i];
    return 0;
  }

  void show() {
    if (on_show_) on_show_(pixels_);
  }

  // Emulator hook: register callback to receive full pixel buffer on show()
  void setShowCallback(std::function<void(const std::vector<uint32_t>&)> cb) { on_show_ = std::move(cb); }
  // Resize buffer (used by emulator to create the global instance with known size)
  void resize(uint16_t n) { count = n; pixels_.assign(n, 0); }

private:
  uint32_t applyBrightness(uint32_t c) const {
    if (brightness_ >= 255) return c;
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;
    r = uint16_t(r) * brightness_ / 255;
    g = uint16_t(g) * brightness_ / 255;
    b = uint16_t(b) * brightness_ / 255;
    return Color(r,g,b);
  }

  uint16_t count;
  std::vector<uint32_t> pixels_;
  uint8_t brightness_;
  std::function<void(const std::vector<uint32_t>&)> on_show_;
};
