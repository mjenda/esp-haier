#pragma once

#include "esphome.h"
#include <array>

#include "constants.h"
#include "status.h"
#include "utility.h"


class Control {
public:
  Control(const Status &status, const ClimateCall &call)
      : status_(status), call_(call) {
    UpdateFromStatus();
    UpdateFromHomeAssitant();
  }

  void Send() { sendData(control_command_.data(), control_command_.size()); }

private:
  void UpdateFromStatus() {
    SetPowerControl(status_.GetPowerStatus());
    SetHvacModeControl(status_.GetHvacModeStatus());
    SetPurifyControl(status_.GetPurifyStatus());
    SetQuietModeControl(status_.GetQuietModeStatus());
    SetFastModeControl(status_.GetFastModeStatus());
    SetFanSpeedControl(status_.GetFanSpeedStatus());
    SetHorizontalSwingControl(status_.GetHorizontalSwingStatus());
    SetVerticalSwingControl(status_.GetVerticalSwingStatus());
    SetTemperatureSetpointControl(status_.GetTemperatureSetpointStatus());
  }

  void UpdateFromHomeAssitant() {
    HandleClimateMode();
    HandleFanSpeedMode();
    HandleSwingMode();
    HandleTargetTemperature();
  }

  void HandleClimateMode() {
    auto mode = call_.get_mode();

    if (!mode)
      return;

    ESP_LOGD("EspHaier Control", "mode = %d", *mode);

    switch (*mode) {
    case CLIMATE_MODE_OFF:
      SetPowerControl(!status_.GetPowerStatus());
      break;

    case CLIMATE_MODE_HEAT_COOL:
      SetPowerControl(true);
      SetHvacModeControl(MODE_AUTO);
      SetFanSpeedControl(status_.GetClimateModeFanSpeed());
      SetTemperatureSetpointControl(status_.GetClimateModeSetpoint());
      break;

    case CLIMATE_MODE_HEAT:
      SetPowerControl(true);
      SetHvacModeControl(MODE_HEAT);
      SetFanSpeedControl(status_.GetClimateModeFanSpeed());
      SetTemperatureSetpointControl(status_.GetClimateModeSetpoint());
      break;

    case CLIMATE_MODE_DRY:
      SetPowerControl(true);
      SetHvacModeControl(MODE_DRY);
      SetFanSpeedControl(status_.GetClimateModeFanSpeed());
      SetTemperatureSetpointControl(status_.GetClimateModeSetpoint());
      break;

    case CLIMATE_MODE_FAN_ONLY:
      SetPowerControl(true);
      SetHvacModeControl(MODE_FAN);
      SetFanSpeedControl(status_.GetFanModeFanSpeed());
      SetTemperatureSetpointControl(status_.GetFanModeSetpoint());
      break;

    case CLIMATE_MODE_COOL:
      SetPowerControl(true);
      SetHvacModeControl(MODE_COOL);
      SetFanSpeedControl(status_.GetClimateModeFanSpeed());
      SetTemperatureSetpointControl(status_.GetClimateModeSetpoint());
      break;

    default:
      ESP_LOGE("EspHaier Control", "Unhandled mode : %d", *mode);
      break;
    }
  }

  void HandleFanSpeedMode() {
    auto fan_mode = call_.get_fan_mode();

    if (!fan_mode)
      return;

    ESP_LOGD("EspHaier Control", "fan_mode = %d", *fan_mode);

    switch (*fan_mode) {
    case CLIMATE_FAN_AUTO:
      SetFanSpeedControl(FAN_AUTO);
      break;
    case CLIMATE_FAN_LOW:
      SetFanSpeedControl(FAN_LOW);
      break;
    case CLIMATE_FAN_MEDIUM:
      SetFanSpeedControl(FAN_MID);
      break;
    case CLIMATE_FAN_HIGH:
      SetFanSpeedControl(FAN_HIGH);
      break;
    default:
      ESP_LOGE("EspHaier Control", "Unhandled fan mode : %d", *fan_mode);
      break;
    }
  }

  void HandleSwingMode() {
    auto swing_mode = call_.get_swing_mode();

    if (!swing_mode)
      return;

    ESP_LOGD("EspHaier Control", "swing_mode = %d", *swing_mode);

    switch (*swing_mode) {
    case CLIMATE_SWING_OFF:
      SetHorizontalSwingControl(HORIZONTAL_SWING_CENTER);
      SetVerticalSwingControl(VERTICAL_SWING_CENTER);
      break;
    case CLIMATE_SWING_VERTICAL:
      SetHorizontalSwingControl(HORIZONTAL_SWING_CENTER);
      SetVerticalSwingControl(VERTICAL_SWING_AUTO);
      break;
    case CLIMATE_SWING_HORIZONTAL:
      SetHorizontalSwingControl(HORIZONTAL_SWING_AUTO);
      SetVerticalSwingControl(VERTICAL_SWING_CENTER);
      break;
    case CLIMATE_SWING_BOTH:
      SetHorizontalSwingControl(HORIZONTAL_SWING_AUTO);
      SetVerticalSwingControl(VERTICAL_SWING_AUTO);
      break;
    }
  }

  void HandleTargetTemperature() {
    auto temp = call_.get_target_temperature();

    if (!temp)
      return;

    ESP_LOGD("EspHaier Control", "*call.get_target_temperature() = %f", *temp);

    SetPointOffset(*temp);
  }

  void SetHvacModeControl(byte mode) {
    control_command_[MODE_OFFSET] &= ~MODE_MSK;
    control_command_[MODE_OFFSET] |= mode;
  }

  void SetTemperatureSetpointControl(byte temp) {
    control_command_[SET_POINT_OFFSET] = temp;
  }

  void SetFanSpeedControl(byte fan_mode) {
    control_command_[MODE_OFFSET] &= ~FAN_MSK;
    control_command_[MODE_OFFSET] |= fan_mode;
  }

  void SetHorizontalSwingControl(byte swing_mode) {
    control_command_[HORIZONTAL_SWING_OFFSET] = swing_mode;
  }

  void SetVerticalSwingControl(byte swing_mode) {
    control_command_[VERTICAL_SWING_OFFSET] = swing_mode;
  }

  void SetQuietModeControl(bool quiet_mode) {
    ApplyStatusDataField(quiet_mode, QUIET_BIT);
  }

  void SetPurifyControl(bool purify_mode) {
    ApplyStatusDataField(purify_mode, PURIFY_BIT);
  }

  void SetPowerControl(bool power_mode) {
    ApplyStatusDataField(power_mode, POWER_BIT);
  }

  void SetFastModeControl(bool fast_mode) {
    ApplyStatusDataField(fast_mode, AUTO_FAN_MAX_BIT);
  }

  void SetPointOffset(float temp) {
    control_command_[SET_POINT_OFFSET] = (uint16)temp - 16;
  }

  void ApplyStatusDataField(bool state, uint8_t field) {
    byte msk = (0x01 << field);

    if (state) {
      control_command_[STATUS_DATA_OFFSET] |= msk;
    } else {
      msk = ~msk;
      control_command_[STATUS_DATA_OFFSET] &= msk;
    }
  }

  const Status &status_;
  const ClimateCall &call_;

  std::array<byte, 25> control_command_{
      {0xFF, 0xFF, 0x14, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x01, 0x60, 0x01, 0x09, 0x08, 0x25, 0x00, 0x02, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
};