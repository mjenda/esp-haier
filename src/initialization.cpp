#include "initialization.h"

#include "esphome.h"

#include "utility.h"

using esphome::esp_log_printf_;

void Initialization::Initialize() {
  Send(initialization_1);
  Send(initialization_2);
}

void Initialization::Send(const InitializationType &initialization) {
  ::delay(1000);
  Serial.write(initialization.data(), initialization.size());
  auto raw = getHex(initialization);
  ESP_LOGD("EspHaier Initialization", "initialization: %s ", raw.c_str());
}