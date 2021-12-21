#pragma once

#include <array>

#include "esphome.h"

#include "constants.h"
#include "utility.h"
class Status {
public:
  byte GetHvacModeStatus() const;
  byte GetTemperatureSetpointStatus() const;

  byte GetFanSpeedStatus() const;
  byte GetHorizontalSwingStatus() const;
  byte GetVerticalSwingStatus() const;
  byte GetClimateModeFanSpeed() const;
  byte GetClimateModeSetpoint() const;
  byte GetFanModeFanSpeed() const;
  byte GetFanModeSetpoint() const;
  bool GetPurifyStatus() const;
  bool GetPowerStatus() const;
  bool GetQuietModeStatus() const;
  bool GetFastModeStatus() const;
  esphome::climate::ClimateMode GetMode() const;
  esphome::climate::ClimateFanMode GetFanMode() const;
  esphome::climate::ClimateSwingMode GetSwingMode() const;
  float GetCurrentTemperature() const;
  float GetTargetTemperature() const;
  bool GetFirstStatusReceived() const;

  void LogStatus();
  bool OnPendingData();
  void SendPoll() const;

private:
  bool GetStatusDataField(byte bit) const;
  bool ValidateChecksum() const;
  bool ValidateTemperature() const;
  void UpdateStatus(const StatusMessageType &data);
  void LogChangedBytes();
  void PrintDebug();

  byte climate_mode_fan_speed_ = FanMode::FanAuto;
  byte climate_mode_setpoint_ = 0x0A;
  byte fan_mode_fan_speed_ = FanMode::FanHigh;
  byte fan_mode_setpoint_ = 0x08;
  bool first_status_received_ = false;

  StatusMessageType status_ = GetStatusMessage();
  StatusMessageType previous_status_ = GetStatusMessage();
  PollMessageType poll_ = GetPollMessage();
};