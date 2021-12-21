#pragma once

#include "esphome.h"
#include <array>

class Initialization {
public:
  void Initialize() {
    Send(initialization_1);
    Send(initialization_2);
  }

private:
  void Send(const std::array<byte, 13> &initialization) {
    delay(1000);
    Serial.write(initialization.data(), initialization.size());
    auto raw = getHex(initialization.data(), initialization.size());
    ESP_LOGD("EspHaier Initialization", "initialization: %s ", raw.c_str());
  }

  std::array<byte, 13> initialization_1{INITIALIZATION_1};
  std::array<byte, 13> initialization_2{INITIALIZATION_2};
};