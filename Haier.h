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
#include "status.h"
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
    status_.OnStatusReceived(std::bind(&Haier::onStatusReceived, this));
  }

  void update() override { status_.SendPoll(); }

  void onStatusReceived() {
    status_.EspLog();

    if (!status_.ValidateChecksum() || !status_.ValidateTemperature()) {
      return;
    }

    control_command_.UpdateFromStatus(status_);

    status_.StorePreviousStatusForDebug();

    // Update home assistant component
    mode = status_.GetMode();
    fan_mode = status_.GetFanMode();
    swing_mode = status_.GetSwingMode();
    current_temperature = status_.GetCurrentTemperature();
    target_temperature = status_.GetTargetTemperature();

    this->publish_state();
  }

  void control(const ClimateCall &call) override {
    ClimateMode new_mode;
    bool new_control_cmd = false;

    ESP_LOGD("EspHaier Control", "Control call");

    if (!status_.GetFirstStatusReceived()) {
      ESP_LOGD("EspHaier Control", "No action, first poll answer not received");
      return;
    }

    if (call.get_mode().has_value()) {
      // User requested mode change
      new_mode = *call.get_mode();

      ESP_LOGD("EspHaier Control", "*call.get_mode() = %d", new_mode);

      switch (new_mode) {
      case CLIMATE_MODE_OFF:
        control_command_.SetPowerControl(!status_.GetPowerStatus());
        sendData(control_command_.Data(), control_command_.Size());
        break;

      case CLIMATE_MODE_HEAT_COOL:
        control_command_.SetPowerControl(true);
        control_command_.SetHvacModeControl(MODE_AUTO);

        // Recover fan_speed and setpoint (when switching to fan_only they are
        // "lost")
        control_command_.SetFanSpeedControl(status_.GetClimateModeFanSpeed());
        control_command_.SetTemperatureSetpointControl(
            status_.GetClimateModeSetpoint());

        sendData(control_command_.Data(), control_command_.Size());
        break;

      case CLIMATE_MODE_HEAT:
        control_command_.SetPowerControl(true);
        control_command_.SetHvacModeControl(MODE_HEAT);

        // Recover fan_speed and setpoint (when switching to fan_only they are
        // "lost")
        control_command_.SetFanSpeedControl(status_.GetClimateModeFanSpeed());
        control_command_.SetTemperatureSetpointControl(
            status_.GetClimateModeSetpoint());

        sendData(control_command_.Data(), control_command_.Size());
        break;

      case CLIMATE_MODE_DRY:
        control_command_.SetPowerControl(true);
        control_command_.SetHvacModeControl(MODE_DRY);

        // Recover fan_speed and setpoint (when switching to fan_only they are
        // "lost")
        control_command_.SetFanSpeedControl(status_.GetClimateModeFanSpeed());
        control_command_.SetTemperatureSetpointControl(
            status_.GetClimateModeSetpoint());

        sendData(control_command_.Data(), control_command_.Size());
        break;

      case CLIMATE_MODE_FAN_ONLY:
        control_command_.SetPowerControl(true);
        control_command_.SetHvacModeControl(MODE_FAN);

        // Recover fan_speed and setpoint (fan_only values are "special")
        control_command_.SetFanSpeedControl(status_.GetFanModeFanSpeed());
        control_command_.SetTemperatureSetpointControl(
            status_.GetFanModeSetpoint());

        sendData(control_command_.Data(), control_command_.Size());
        break;

      case CLIMATE_MODE_COOL:
        control_command_.SetPowerControl(true);
        control_command_.SetHvacModeControl(MODE_COOL);

        // Recover fan_speed and setpoint (when switching to fan_only they are
        // "lost")
        control_command_.SetFanSpeedControl(status_.GetClimateModeFanSpeed());
        control_command_.SetTemperatureSetpointControl(
            status_.GetClimateModeSetpoint());

        sendData(control_command_.Data(), control_command_.Size());
        break;

      case CLIMATE_MODE_AUTO:
      default:
        break;
      }

      // Publish updated state
      mode = new_mode;
      this->publish_state();
    }

    // Set fan speed
    if (call.get_fan_mode().has_value()) {
      switch (call.get_fan_mode().value()) {
      case CLIMATE_FAN_LOW:
        control_command_.SetFanSpeedControl(FAN_LOW);
        break;
      case CLIMATE_FAN_MIDDLE:
        control_command_.SetFanSpeedControl(FAN_MID);
        break;
      case CLIMATE_FAN_MEDIUM:
        control_command_.SetFanSpeedControl(FAN_MID);
        break;
      case CLIMATE_FAN_HIGH:
        control_command_.SetFanSpeedControl(FAN_HIGH);
        break;
      case CLIMATE_FAN_AUTO:
        control_command_.SetFanSpeedControl(FAN_AUTO);
        break;
      case CLIMATE_FAN_ON:
      case CLIMATE_FAN_OFF:
      case CLIMATE_FAN_FOCUS:
      case CLIMATE_FAN_DIFFUSE:
      default:
        break;
      }
      sendData(control_command_.Data(), control_command_.Size());
    }

    // Set swing mode
    if (call.get_swing_mode().has_value()) {
      switch (call.get_swing_mode().value()) {
      case CLIMATE_SWING_OFF:
        // When not auto we decide to set it to the center
        control_command_.SetHorizontalSwingControl(HORIZONTAL_SWING_CENTER);
        // When not auto we decide to set it to the center
        control_command_.SetVerticalSwingControl(VERTICAL_SWING_CENTER);
        break;
      case CLIMATE_SWING_VERTICAL:
        // When not auto we decide to set it to the center
        control_command_.SetHorizontalSwingControl(HORIZONTAL_SWING_CENTER);
        control_command_.SetVerticalSwingControl(VERTICAL_SWING_AUTO);
        break;
      case CLIMATE_SWING_HORIZONTAL:
        control_command_.SetHorizontalSwingControl(HORIZONTAL_SWING_AUTO);
        // When not auto we decide to set it to the center
        control_command_.SetVerticalSwingControl(VERTICAL_SWING_CENTER);
        break;
      case CLIMATE_SWING_BOTH:
        control_command_.SetHorizontalSwingControl(HORIZONTAL_SWING_AUTO);
        control_command_.SetVerticalSwingControl(VERTICAL_SWING_AUTO);
        break;
      }
      sendData(control_command_.Data(), control_command_.Size());
    }

    if (call.get_target_temperature().has_value()) {
      float temp = *call.get_target_temperature();
      ESP_LOGD("EspHaier Control", "*call.get_target_temperature() = %f", temp);
      control_command_.SetPointOffset(temp);
      sendData(control_command_.Data(), control_command_.Size());
      target_temperature = temp;
      this->publish_state();
    }
  }

protected:
  ClimateTraits traits() override {
    auto traits = climate::ClimateTraits();
    traits.set_supported_modes(
        {climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT_COOL,
         climate::CLIMATE_MODE_HEAT, climate::CLIMATE_MODE_COOL,
         climate::CLIMATE_MODE_DRY, climate::CLIMATE_MODE_FAN_ONLY});

    traits.set_supported_fan_modes(
        {climate::CLIMATE_FAN_ON, climate::CLIMATE_FAN_OFF,
         climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW,
         climate::CLIMATE_FAN_MEDIUM, climate::CLIMATE_FAN_MIDDLE,
         climate::CLIMATE_FAN_HIGH});

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
  ControlCommand control_command_;
  Status status_;
};
