#pragma once

/**
 * Create by Miguel Ángel López on 20/07/19
 * Modified by Alba Prades on 21/07/20
 * Modified by Alba Prades on 13/08/20
 * Modified by Alba Prades on 25/08/20: Added fan, dry and swing features
 *      Added modes
 **/

#include "esphome.h"
#include <string>

#include "constants.h"
#include "utility.h"
#include "initialization.h"
#include "control_command.h"
#include "status.h"

using namespace esphome;
using namespace esphome::climate;

class Haier : public Climate, public PollingComponent {

private:
  byte poll[15] = {0xFF, 0xFF, 0x0A, 0x40, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x01, 0x4D, 0x01, 0x99, 0xB3, 0xB4};
  byte power_command[17] = {0xFF, 0xFF, 0x0C, 0x40, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x01, 0x5D, 0x01,
                            0x00, 0x01, 0xAC, 0xBD, 0xFB};
  ControlCommand control_command_;
  Status status_;

  byte climate_mode_fan_speed = FAN_AUTO;
  byte climate_mode_setpoint = 0x0A;

  byte fan_mode_fan_speed = FAN_HIGH;
  byte fan_mode_setpoint = 0x08;

  bool first_status_received = false;

public:
  Haier() : PollingComponent(5000 /*5 sec*/) {}

  void setup() override {
    Serial.begin(9600);
    Initialization().Initialize();
  }

  void loop() override {
    std::array<byte, 47> data;
    if (Serial.available() > 0) {
      if (Serial.read() != 255)
        return;
      if (Serial.read() != 255)
        return;

      data[0] = 255;
      data[1] = 255;

      Serial.readBytes(data.data() + 2, data.size() - 2);

      // If is a status response
      if (data[COMMAND_OFFSET] == RESPONSE_POLL) {
        status_.Update(data);
        parseStatus();
      }
    }
  }

  void update() override {

    Serial.write(poll, sizeof(poll));
    auto raw = getHex(poll, sizeof(poll));
    ESP_LOGD("Haier", "POLL: %s ", raw.c_str());
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

public:
  void parseStatus() {
    status_.EspLog();
    byte check = getChecksum(status_.Data(), status_.Size());

    if (!status_.ValidateChecksum()) {
      return;
    }

    current_temperature = status_.GetCurrentTemperature();
    target_temperature = status_.GetTargetTemperature();

    if (!status_.ValidateTemperature()) {
      return;
    }

    // Read all the info from the status message and update values in control
    // message so the next message is updated This is usefull if there are
    // manual changes with the remote control
    control_command_.SetPowerControl(status_.GetPowerStatus());
    control_command_.SetHvacModeControl(status_.GetHvacModeStatus());
    control_command_.SetPurifyControl(status_.GetPurifyStatus());
    control_command_.SetQuietModeControl(status_.GetQuietModeStatus());
    control_command_.SetFastModeControl(status_.GetFastModeStatus());
    control_command_.SetFanSpeedControl(status_.GetFanSpeedStatus());
    control_command_.SetHorizontalSwingControl(status_.GetHorizontalSwingStatus());
    control_command_.SetVerticalSwingControl(status_.GetVerticalSwingStatus());
    control_command_.SetTemperatureSetpointControl(status_.GetTemperatureSetpointStatus());

    if (status_.GetHvacModeStatus() == MODE_FAN) {
      fan_mode_fan_speed = status_.GetFanSpeedStatus();
      fan_mode_setpoint = status_.GetTemperatureSetpointStatus();
    } else {
      climate_mode_fan_speed = status_.GetFanSpeedStatus();
      climate_mode_setpoint = status_.GetTemperatureSetpointStatus();
    }

    // Flag to enable modifications from UI as we now know the status of the A/C
    first_status_received = true;

    // DEBUG DATA, uncomment what's needed
    // ESP_LOGW("Debug", "Power Status = 0x%X", GetPowerStatus());
    // ESP_LOGW("Debug", "HVAC Mode = 0x%X", GetHvacModeStatus());
    // ESP_LOGW("Debug", "Purify status = 0x%X", GetPurifyStatus());
    // ESP_LOGW("Debug", "Quiet mode Status = 0x%X", GetQuietModeStatus());
    // ESP_LOGW("Debug", "Fast mode Status = 0x%X", GetFastModeStatus());
    // ESP_LOGW("Debug", "Fan speed Status = 0x%X", GetFanSpeedStatus());
    // ESP_LOGW("Debug", "Horizontal Swing Status = 0x%X",
    // GetHorizontalSwingStatus()); ESP_LOGW("Debug", "Vertical Swing Status =
    // 0x%X", GetVerticalSwingStatus()); ESP_LOGW("Debug", "Set Point Status =
    // 0x%X", GetTemperatureSetpointStatus());
    status_.CompareStatusByte();

    // Update home assistant component

    if (status_.GetPowerStatus() == false) {
      mode = CLIMATE_MODE_OFF;
    } else {
      // Check current hvac mode
      switch (status_.GetHvacModeStatus()) {
      case MODE_COOL:
        mode = CLIMATE_MODE_COOL;
        break;
      case MODE_HEAT:
        mode = CLIMATE_MODE_HEAT;
        break;
      case MODE_DRY:
        mode = CLIMATE_MODE_DRY;
        break;
      case MODE_FAN:
        mode = CLIMATE_MODE_FAN_ONLY;
        break;
      case MODE_AUTO:
      default:
        mode = CLIMATE_MODE_HEAT_COOL;
      }

      // Get fan speed
      // If "quiet mode" is set we will read it as "fan low"
      if (status_.GetQuietModeStatus() == true) {
        fan_mode = CLIMATE_FAN_LOW;
      }
      // If we detect that fast mode is on the we read it as "fan high"
      else if (status_.GetFastModeStatus() == true) {
        fan_mode = CLIMATE_FAN_HIGH;
      } else {
        // No quiet or fast so we read the actual fan speed.
        switch (status_.GetFanSpeedStatus()) {
        case FAN_AUTO:
          fan_mode = CLIMATE_FAN_AUTO;
          break;
        case FAN_MID:
          fan_mode = CLIMATE_FAN_MEDIUM;
          break;
          // case FAN_MIDDLE:
          //    fan_mode = CLIMATE_FAN_MIDDLE;
          //    break;
        case FAN_LOW:
          fan_mode = CLIMATE_FAN_LOW;
          break;
        case FAN_HIGH:
          fan_mode = CLIMATE_FAN_HIGH;
          break;
        default:
          fan_mode = CLIMATE_FAN_AUTO;
        }
      }

      // Check the status of the swings (vertical and horizontal and translate
      // according component configuration
      if ((status_.GetHorizontalSwingStatus() == HORIZONTAL_SWING_AUTO) &&
          (status_.GetVerticalSwingStatus() == VERTICAL_SWING_AUTO)) {
        swing_mode = CLIMATE_SWING_BOTH;
      } else if (status_.GetHorizontalSwingStatus() == HORIZONTAL_SWING_AUTO) {
        swing_mode = CLIMATE_SWING_HORIZONTAL;
      } else if (status_.GetVerticalSwingStatus() == VERTICAL_SWING_AUTO) {
        swing_mode = CLIMATE_SWING_VERTICAL;
      } else {
        swing_mode = CLIMATE_SWING_OFF;
      }
    }

    this->publish_state();
  }

  void control(const ClimateCall &call) override {
    ClimateMode new_mode;
    bool new_control_cmd = false;

    ESP_LOGD("Control", "Control call");

    if (first_status_received == false) {
      ESP_LOGD("Control", "No action, first poll answer not received");
      return;
    }

    if (call.get_mode().has_value()) {
      // User requested mode change
      new_mode = *call.get_mode();

      ESP_LOGD("Control", "*call.get_mode() = %d", new_mode);

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
        control_command_.SetFanSpeedControl(climate_mode_fan_speed);
        control_command_.SetTemperatureSetpointControl(climate_mode_setpoint);

        sendData(control_command_.Data(), control_command_.Size());
        break;

      case CLIMATE_MODE_HEAT:
        control_command_.SetPowerControl(true);
        control_command_.SetHvacModeControl(MODE_HEAT);

        // Recover fan_speed and setpoint (when switching to fan_only they are
        // "lost")
        control_command_.SetFanSpeedControl(climate_mode_fan_speed);
        control_command_.SetTemperatureSetpointControl(climate_mode_setpoint);

        sendData(control_command_.Data(), control_command_.Size());
        break;

      case CLIMATE_MODE_DRY:
        control_command_.SetPowerControl(true);
        control_command_.SetHvacModeControl(MODE_DRY);

        // Recover fan_speed and setpoint (when switching to fan_only they are
        // "lost")
        control_command_.SetFanSpeedControl(climate_mode_fan_speed);
        control_command_.SetTemperatureSetpointControl(climate_mode_setpoint);

        sendData(control_command_.Data(), control_command_.Size());
        break;

      case CLIMATE_MODE_FAN_ONLY:
        control_command_.SetPowerControl(true);
        control_command_.SetHvacModeControl(MODE_FAN);

        // Recover fan_speed and setpoint (fan_only values are "special")
        control_command_.SetFanSpeedControl(fan_mode_fan_speed);
        control_command_.SetTemperatureSetpointControl(fan_mode_setpoint);

        sendData(control_command_.Data(), control_command_.Size());
        break;

      case CLIMATE_MODE_COOL:
        control_command_.SetPowerControl(true);
        control_command_.SetHvacModeControl(MODE_COOL);

        // Recover fan_speed and setpoint (when switching to fan_only they are
        // "lost")
        control_command_.SetFanSpeedControl(climate_mode_fan_speed);
        control_command_.SetTemperatureSetpointControl(climate_mode_setpoint);

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
      ESP_LOGD("Control", "*call.get_target_temperature() = %f", temp);
      control_command_.SetPointOffset(temp);
      sendData(control_command_.Data(), control_command_.Size());
      target_temperature = temp;
      this->publish_state();
    }
  }
};
