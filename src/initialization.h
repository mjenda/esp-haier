#pragma once

#include <array>
#include "esphome.h"

class Initialization {
public:
  void Initialize() {
    SendInitialize(initialization_1);
    SendInitialize(initialization_2);
  }

private:
  void SendInitialize(const std::array<byte, 13> &initialization) {
    delay(1000);
    Serial.write(initialization.data(), initialization.size());
    auto raw = getHex(initialization.data(), initialization.size());
    ESP_LOGD("EspHaier Initialization", "initialization: %s ", raw.c_str());
  }

  std::array<byte, 13> initialization_1{{0xFF, 0xFF, 0x0A, 0x0, 0x0, 0x0, 0x0,
                                         0x0, 0x00, 0x61, 0x00, 0x07, 0x72}};
  std::array<byte, 13> initialization_2{{0xFF, 0xFF, 0x08, 0x40, 0x0, 0x0, 0x0,
                                         0x0, 0x0, 0x70, 0xB8, 0x86, 0x41}};
};