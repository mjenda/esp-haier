#pragma once

#include "esphome.h"
#include <functional>
#include <string>

#include "constants.h"
#include "control_command.h"
#include "initialization.h"
#include "status_controller.h"
#include "utility.h"

using namespace esphome;
using namespace esphome::climate;

class Haier : public Climate, public PollingComponent {
public:
  Haier() : PollingComponent(5000 /*5 sec*/) {}

  void setup() override {
    Serial.begin(9600);
    Initialization().Initialize();
  }

  void loop() override {
    if (!status_.OnPendingData())
      return;

    status_.LogStatus();

    Climate::mode = status_.GetMode();
    Climate::fan_mode = status_.GetFanMode();
    Climate::swing_mode = status_.GetSwingMode();
    Climate::current_temperature = status_.GetCurrentTemperature();
    Climate::target_temperature = status_.GetTargetTemperature();
    Climate::publish_state();
  }

  void update() override { status_.SendPoll(); }

  void control(const ClimateCall &call) override {
    ESP_LOGD("EspHaier Control", "Control call");

    if (!status_.GetFirstStatusReceived()) {
      ESP_LOGD("EspHaier Control", "No action, first poll answer not received");
      return;
    }

    ControlCommand(status_, call).Send();
  }

protected:
  ClimateTraits traits() override {
    auto traits = climate::ClimateTraits();
    traits.set_supported_modes(
        {climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT_COOL,
         climate::CLIMATE_MODE_HEAT, climate::CLIMATE_MODE_COOL,
         climate::CLIMATE_MODE_DRY, climate::CLIMATE_MODE_FAN_ONLY});

    traits.set_supported_fan_modes(
        {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW,
         climate::CLIMATE_FAN_MEDIUM, climate::CLIMATE_FAN_HIGH});

    traits.set_visual_min_temperature(MIN_SET_TEMPERATURE);
    traits.set_visual_max_temperature(MAX_SET_TEMPERATURE);
    traits.set_visual_temperature_step(1.0f);
    traits.set_supports_current_temperature(true);

    traits.set_supported_swing_modes(
        {climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_BOTH,
         climate::CLIMATE_SWING_VERTICAL, climate::CLIMATE_SWING_HORIZONTAL});
    return traits;
  }

private:
  StatusController status_;
};
