#pragma once

#include "esphome.h"

#include "control.h"
#include "initialization.h"
#include "status.h"

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

    Control(status_, call).Send();
  }

protected:
  ClimateTraits traits() override {
    auto traits = climate::ClimateTraits();
    traits.set_supported_modes(
        {ClimateMode::CLIMATE_MODE_OFF, ClimateMode::CLIMATE_MODE_HEAT_COOL,
         ClimateMode::CLIMATE_MODE_HEAT, ClimateMode::CLIMATE_MODE_COOL,
         ClimateMode::CLIMATE_MODE_DRY, ClimateMode::CLIMATE_MODE_FAN_ONLY});

    traits.set_supported_fan_modes(
        {ClimateFanMode::CLIMATE_FAN_AUTO, ClimateFanMode::CLIMATE_FAN_LOW,
         ClimateFanMode::CLIMATE_FAN_MEDIUM, ClimateFanMode::CLIMATE_FAN_HIGH});

    traits.set_visual_min_temperature(TempConstraints::MinSetTemperature);
    traits.set_visual_max_temperature(TempConstraints::MaxSetTemperature);
    traits.set_visual_temperature_step(1.0f);
    traits.set_supports_current_temperature(true);

    traits.set_supported_swing_modes(
        {ClimateSwingMode::CLIMATE_SWING_OFF,
         ClimateSwingMode::CLIMATE_SWING_BOTH,
         ClimateSwingMode::CLIMATE_SWING_VERTICAL,
         ClimateSwingMode::CLIMATE_SWING_HORIZONTAL});
    return traits;
  }

private:
  Status status_;
};
