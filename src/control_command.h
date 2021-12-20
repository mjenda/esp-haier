#pragma once

#include <array>

#include "constants.h"
#include "status_controller.h"

class ControlCommand {
public:
  void UpdateFromStatus(const StatusController &status) {
    SetPowerControl(status.GetPowerStatus());
    SetHvacModeControl(status.GetHvacModeStatus());
    SetPurifyControl(status.GetPurifyStatus());
    SetQuietModeControl(status.GetQuietModeStatus());
    SetFastModeControl(status.GetFastModeStatus());
    SetFanSpeedControl(status.GetFanSpeedStatus());
    SetHorizontalSwingControl(status.GetHorizontalSwingStatus());
    SetVerticalSwingControl(status.GetVerticalSwingStatus());
    SetTemperatureSetpointControl(status.GetTemperatureSetpointStatus());
  }

  void HandleClimateMode(optional<ClimateMode> mode,
                         const StatusController &status) {
    if (!mode)
      return;

    ESP_LOGD("EspHaier Control", "mode = %d", *mode);

    switch (*mode) {
    case CLIMATE_MODE_OFF:
      SetPowerControl(!status.GetPowerStatus());
      break;

    case CLIMATE_MODE_HEAT_COOL:
      SetPowerControl(true);
      SetHvacModeControl(MODE_AUTO);

      // Recover fan_speed and setpoint (when switching to fan_only they are
      // "lost")
      SetFanSpeedControl(status.GetClimateModeFanSpeed());
      SetTemperatureSetpointControl(status.GetClimateModeSetpoint());
      break;

    case CLIMATE_MODE_HEAT:
      SetPowerControl(true);
      SetHvacModeControl(MODE_HEAT);

      // Recover fan_speed and setpoint (when switching to fan_only they are
      // "lost")
      SetFanSpeedControl(status.GetClimateModeFanSpeed());
      SetTemperatureSetpointControl(status.GetClimateModeSetpoint());
      break;

    case CLIMATE_MODE_DRY:
      SetPowerControl(true);
      SetHvacModeControl(MODE_DRY);

      // Recover fan_speed and setpoint (when switching to fan_only they are
      // "lost")
      SetFanSpeedControl(status.GetClimateModeFanSpeed());
      SetTemperatureSetpointControl(status.GetClimateModeSetpoint());
      break;

    case CLIMATE_MODE_FAN_ONLY:
      SetPowerControl(true);
      SetHvacModeControl(MODE_FAN);

      // Recover fan_speed and setpoint (fan_only values are "special")
      SetFanSpeedControl(status.GetFanModeFanSpeed());
      SetTemperatureSetpointControl(status.GetFanModeSetpoint());
      break;

    case CLIMATE_MODE_COOL:
      SetPowerControl(true);
      SetHvacModeControl(MODE_COOL);

      // Recover fan_speed and setpoint (when switching to fan_only they are
      // "lost")
      SetFanSpeedControl(status.GetClimateModeFanSpeed());
      SetTemperatureSetpointControl(status.GetClimateModeSetpoint());
      break;

    default:
      ESP_LOGE("EspHaier Control", "Unhandled mode : %d", *mode);
      break;
    }
  }

  void HandleFanSpeedMode(optional<ClimateFanMode> fan_mode) {
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

  void HandleSwingMode(optional<ClimateSwingMode> swing_mode) {
    if (!swing_mode)
      return;

    ESP_LOGD("EspHaier Control", "swing_mode = %d", *swing_mode);

    switch (*swing_mode) {
    case CLIMATE_SWING_OFF:
      // When not auto we decide to set it to the center
      SetHorizontalSwingControl(HORIZONTAL_SWING_CENTER);
      // When not auto we decide to set it to the center
      SetVerticalSwingControl(VERTICAL_SWING_CENTER);
      break;
    case CLIMATE_SWING_VERTICAL:
      // When not auto we decide to set it to the center
      SetHorizontalSwingControl(HORIZONTAL_SWING_CENTER);
      SetVerticalSwingControl(VERTICAL_SWING_AUTO);
      break;
    case CLIMATE_SWING_HORIZONTAL:
      SetHorizontalSwingControl(HORIZONTAL_SWING_AUTO);
      // When not auto we decide to set it to the center
      SetVerticalSwingControl(VERTICAL_SWING_CENTER);
      break;
    case CLIMATE_SWING_BOTH:
      SetHorizontalSwingControl(HORIZONTAL_SWING_AUTO);
      SetVerticalSwingControl(VERTICAL_SWING_AUTO);
      break;
    }
  }

  void HandleTargetTemperature(optional<float> temp) {
    if (!temp)
      return;

    ESP_LOGD("EspHaier Control", "*call.get_target_temperature() = %f", *temp);

    SetPointOffset(*temp);
  }

  void Send() { sendData(control_command_.data(), control_command_.size()); }

private:
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
    byte tmp;
    byte msk;

    msk = (0x01 << QUIET_BIT);

    if (quiet_mode == true) {
      control_command_[STATUS_DATA_OFFSET] |= msk;
    } else {
      msk = ~msk;
      control_command_[STATUS_DATA_OFFSET] &= msk;
    }
  }

  void SetPurifyControl(bool purify_mode) {
    byte tmp;
    byte msk;

    msk = (0x01 << PURIFY_BIT);

    if (purify_mode == true) {
      control_command_[STATUS_DATA_OFFSET] |= msk;
    } else {
      msk = ~msk;
      control_command_[STATUS_DATA_OFFSET] &= msk;
    }
  }

  void SetPowerControl(bool power_mode) {
    byte tmp;
    byte msk;

    msk = (0x01 << POWER_BIT);

    if (power_mode == true) {
      control_command_[STATUS_DATA_OFFSET] |= msk;
    } else {
      msk = ~msk;
      control_command_[STATUS_DATA_OFFSET] &= msk;
    }
  }

  void SetFastModeControl(bool fast_mode) {
    byte tmp;
    byte msk;

    msk = (0x01 << AUTO_FAN_MAX_BIT);

    if (fast_mode == true) {
      control_command_[STATUS_DATA_OFFSET] |= msk;
    } else {
      msk = ~msk;
      control_command_[STATUS_DATA_OFFSET] &= msk;
    }
  }

  void SetPointOffset(float temp) {
    control_command_[SET_POINT_OFFSET] = (uint16)temp - 16;
  }

  std::array<byte, 25> control_command_{
      {0xFF, 0xFF, 0x14, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x01, 0x60, 0x01, 0x09, 0x08, 0x25, 0x00, 0x02, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
};