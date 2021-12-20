#pragma once

/**
 * Create by Miguel Ángel López on 20/07/19
 * Modified by Alba Prades on 21/07/20
 * Modified by Alba Prades on 13/08/20
 * Modified by Alba Prades on 25/08/20: Added fan, dry and swing features
 *      Added modes
 **/

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
    if (status_.OnPendingData())
      onStatusReceived();
  }

  void update() override { status_.SendPoll(); }

  void onStatusReceived() {
    status_.LogStatus();

    // Update home assistant component
    mode = status_.GetMode();
    fan_mode = status_.GetFanMode();
    swing_mode = status_.GetSwingMode();
    current_temperature = status_.GetCurrentTemperature();
    target_temperature = status_.GetTargetTemperature();

    this->publish_state();
  }

  void control(const ClimateCall &call) override {
    ESP_LOGD("EspHaier Control", "Control call");

    if (!status_.GetFirstStatusReceived()) {
      ESP_LOGD("EspHaier Control", "No action, first poll answer not received");
      return;
    }

    ControlCommand control_command_;

    control_command_.UpdateFromStatus(status_);
    control_command_.HandleClimateMode(call.get_mode(), status_);
    control_command_.HandleFanSpeedMode(call.get_fan_mode());
    control_command_.HandleSwingMode(call.get_swing_mode());
    control_command_.HandleTargetTemperature(call.get_target_temperature());

    control_command_.Send();
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
