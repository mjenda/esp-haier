#pragma once

#include "esphome.h"

class Initialization {
public:
  void Initialize() {
    Send(initialization_1);
    Send(initialization_2);
  }

private:
  void Send(const InitializationType &initialization) {
    delay(1000);
    Serial.write(initialization.data(), initialization.size());
    auto raw = getHex(initialization);
    ESP_LOGD("EspHaier Initialization", "initialization: %s ", raw.c_str());
  }

  InitializationType initialization_1 = GetInitialization1();
  InitializationType initialization_2 = GetInitialization2();
};