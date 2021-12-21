#include "control.h"

#include "esphome.h"

#include "utility.h"

using esphome::esp_log_printf_;
using esphome::climate::ClimateCall;
using esphome::climate::ClimateMode;
using esphome::climate::ClimateFanMode;
using esphome::climate::ClimateSwingMode;

Control::Control(const Status &status, const ClimateCall &call)
    : status_(status), call_(call) {
  UpdateFromStatus();
  UpdateFromHomeAssitant();
}

void Control::Send() { sendData(control_command_); }

void Control::UpdateFromStatus() {
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

void Control::UpdateFromHomeAssitant() {
  HandleClimateMode();
  HandleFanSpeedMode();
  HandleSwingMode();
  HandleTargetTemperature();
}

void Control::HandleClimateMode() {
  auto mode = call_.get_mode();

  if (!mode)
    return;

  ESP_LOGD("EspHaier Control", "mode = %d", *mode);

  switch (*mode) {
  case ClimateMode::CLIMATE_MODE_OFF:
    SetPowerControl(!status_.GetPowerStatus());
    break;

  case ClimateMode::CLIMATE_MODE_HEAT_COOL:
    SetPowerControl(true);
    SetHvacModeControl(AcMode::ModeAuto);
    SetFanSpeedControl(status_.GetClimateModeFanSpeed());
    SetTemperatureSetpointControl(status_.GetClimateModeSetpoint());
    break;

  case ClimateMode::CLIMATE_MODE_HEAT:
    SetPowerControl(true);
    SetHvacModeControl(AcMode::ModeHeat);
    SetFanSpeedControl(status_.GetClimateModeFanSpeed());
    SetTemperatureSetpointControl(status_.GetClimateModeSetpoint());
    break;

  case ClimateMode::CLIMATE_MODE_DRY:
    SetPowerControl(true);
    SetHvacModeControl(AcMode::ModeDry);
    SetFanSpeedControl(status_.GetClimateModeFanSpeed());
    SetTemperatureSetpointControl(status_.GetClimateModeSetpoint());
    break;

  case ClimateMode::CLIMATE_MODE_FAN_ONLY:
    SetPowerControl(true);
    SetHvacModeControl(AcMode::ModeFan);
    SetFanSpeedControl(status_.GetFanModeFanSpeed());
    SetTemperatureSetpointControl(status_.GetFanModeSetpoint());
    break;

  case ClimateMode::CLIMATE_MODE_COOL:
    SetPowerControl(true);
    SetHvacModeControl(AcMode::ModeCool);
    SetFanSpeedControl(status_.GetClimateModeFanSpeed());
    SetTemperatureSetpointControl(status_.GetClimateModeSetpoint());
    break;

  default:
    ESP_LOGE("EspHaier Control", "Unhandled mode : %d", *mode);
    break;
  }
}

void Control::HandleFanSpeedMode() {
  auto fan_mode = call_.get_fan_mode();

  if (!fan_mode)
    return;

  ESP_LOGD("EspHaier Control", "fan_mode = %d", *fan_mode);

  switch (*fan_mode) {
  case ClimateFanMode::CLIMATE_FAN_AUTO:
    SetFanSpeedControl(FanMode::FanAuto);
    break;
  case ClimateFanMode::CLIMATE_FAN_LOW:
    SetFanSpeedControl(FanMode::FanLow);
    break;
  case ClimateFanMode::CLIMATE_FAN_MEDIUM:
    SetFanSpeedControl(FanMode::FanMid);
    break;
  case ClimateFanMode::CLIMATE_FAN_HIGH:
    SetFanSpeedControl(FanMode::FanHigh);
    break;
  default:
    ESP_LOGE("EspHaier Control", "Unhandled fan mode : %d", *fan_mode);
    break;
  }
}

void Control::HandleSwingMode() {
  auto swing_mode = call_.get_swing_mode();

  if (!swing_mode)
    return;

  ESP_LOGD("EspHaier Control", "swing_mode = %d", *swing_mode);

  switch (*swing_mode) {
  case ClimateSwingMode::CLIMATE_SWING_OFF:
    SetHorizontalSwingControl(HorizontalSwingMode::HorizontalSwingCenter);
    SetVerticalSwingControl(VerticalSwingMode::VerticalSwingCenter);
    break;
  case ClimateSwingMode::CLIMATE_SWING_VERTICAL:
    SetHorizontalSwingControl(HorizontalSwingMode::HorizontalSwingCenter);
    SetVerticalSwingControl(VerticalSwingMode::VerticalSwingAuto);
    break;
  case ClimateSwingMode::CLIMATE_SWING_HORIZONTAL:
    SetHorizontalSwingControl(HorizontalSwingMode::HorizontalSwingAuto);
    SetVerticalSwingControl(VerticalSwingMode::VerticalSwingCenter);
    break;
  case ClimateSwingMode::CLIMATE_SWING_BOTH:
    SetHorizontalSwingControl(HorizontalSwingMode::HorizontalSwingAuto);
    SetVerticalSwingControl(VerticalSwingMode::VerticalSwingAuto);
    break;
  }
}

void Control::HandleTargetTemperature() {
  auto temp = call_.get_target_temperature();

  if (!temp)
    return;

  ESP_LOGD("EspHaier Control", "*call.get_target_temperature() = %f", *temp);

  SetPointOffset(*temp);
}

void Control::SetHvacModeControl(byte mode) {
  control_command_[Offset::OffsetMode] &= ~AcMode::ModeMask;
  control_command_[Offset::OffsetMode] |= mode;
}

void Control::SetTemperatureSetpointControl(byte temp) {
  control_command_[Offset::OffsetSetTemperature] = temp;
}

void Control::SetFanSpeedControl(byte fan_mode) {
  control_command_[Offset::OffsetMode] &= ~FanMode::FanMask;
  control_command_[Offset::OffsetMode] |= fan_mode;
}

void Control::SetHorizontalSwingControl(byte swing_mode) {
  control_command_[Offset::OffsetHorizontalSwing] = swing_mode;
}

void Control::SetVerticalSwingControl(byte swing_mode) {
  control_command_[Offset::OffsetVerticalSwing] = swing_mode;
}

void Control::SetQuietModeControl(bool quiet_mode) {
  ApplyStatusDataField(quiet_mode, DataField::DataFieldQuiet);
}

void Control::SetPurifyControl(bool purify_mode) {
  ApplyStatusDataField(purify_mode, DataField::DataFieldPurify);
}

void Control::SetPowerControl(bool power_mode) {
  ApplyStatusDataField(power_mode, DataField::DataFieldPower);
}

void Control::SetFastModeControl(bool fast_mode) {
  ApplyStatusDataField(fast_mode, DataField::DataFieldFanMax);
}

void Control::SetPointOffset(float temp) {
  control_command_[Offset::OffsetSetTemperature] = (uint16)temp - 16;
}

void Control::ApplyStatusDataField(bool state, uint8_t field) {
  byte msk = (0x01 << field);

  if (state) {
    control_command_[Offset::OffsetStatusData] |= msk;
  } else {
    msk = ~msk;
    control_command_[Offset::OffsetStatusData] &= msk;
  }
}