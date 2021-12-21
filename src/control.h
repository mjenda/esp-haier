#pragma once

#include "esphome.h"

#include "constants.h"
#include "status.h"

class Control {
public:
  Control(const Status &status, const esphome::climate::ClimateCall &call);

  void Send();

private:
  void UpdateFromStatus();
  void UpdateFromHomeAssitant();

  void HandleClimateMode();
  void HandleFanSpeedMode();
  void HandleSwingMode();
  void HandleTargetTemperature();

  void SetHvacModeControl(byte mode);
  void SetTemperatureSetpointControl(byte temp);
  void SetFanSpeedControl(byte fan_mode);
  void SetHorizontalSwingControl(byte swing_mode);
  void SetVerticalSwingControl(byte swing_mode);
  void SetQuietModeControl(bool quiet_mode);
  void SetPurifyControl(bool purify_mode);
  void SetPowerControl(bool power_mode);
  void SetFastModeControl(bool fast_mode);
  void SetPointOffset(float temp);
  void ApplyStatusDataField(bool state, uint8_t field);

  const Status &status_;
  const esphome::climate::ClimateCall &call_;
  ControlMessagType control_command_ = GetControlMessage();
};