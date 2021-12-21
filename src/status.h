#pragma once

#include "esphome.h"
#include <array>

#include "constants.h"
#include "utility.h"

class Status {
public:
  byte GetHvacModeStatus() const {
    return status_[Offset::OffsetMode] & AcMode::ModeMask;
  }

  byte GetTemperatureSetpointStatus() const {
    return status_[Offset::OffsetSetTemperature];
  }

  byte GetFanSpeedStatus() const {
    return status_[Offset::OffsetMode] & FanMode::FanMask;
  }

  byte GetHorizontalSwingStatus() const {
    return status_[Offset::OffsetHorizontalSwing];
  }

  byte GetVerticalSwingStatus() const {
    return status_[Offset::OffsetVerticalSwing];
  }

  byte GetClimateModeFanSpeed() const { return climate_mode_fan_speed_; }

  byte GetClimateModeSetpoint() const { return climate_mode_setpoint_; }

  byte GetFanModeFanSpeed() const { return fan_mode_fan_speed_; }

  byte GetFanModeSetpoint() const { return fan_mode_setpoint_; }

  bool GetPurifyStatus() const {
    return GetStatusDataField(DataField::DataFieldPurify);
  }

  bool GetPowerStatus() const {
    return GetStatusDataField(DataField::DataFieldPower);
  }

  bool GetQuietModeStatus() const {
    return GetStatusDataField(DataField::DataFieldQuiet);
  }

  bool GetFastModeStatus() const {
    return GetStatusDataField(DataField::DataFieldFanMax);
  }

  ClimateMode GetMode() const {
    if (!GetPowerStatus())
      return CLIMATE_MODE_OFF;

    switch (GetHvacModeStatus()) {
    case AcMode::ModeCool:
      return CLIMATE_MODE_COOL;
    case AcMode::ModeHeat:
      return CLIMATE_MODE_HEAT;
    case AcMode::ModeDry:
      return CLIMATE_MODE_DRY;
    case AcMode::ModeFan:
      return CLIMATE_MODE_FAN_ONLY;
    case AcMode::ModeAuto:
    default:
      return CLIMATE_MODE_HEAT_COOL;
    }
  }

  ClimateFanMode GetFanMode() const {
    if (!GetPowerStatus())
      return CLIMATE_FAN_OFF;

    if (GetQuietModeStatus())
      return CLIMATE_FAN_LOW;

    if (GetFastModeStatus())
      return CLIMATE_FAN_HIGH;

    switch (GetFanSpeedStatus()) {
    case FanMode::FanAuto:
      return CLIMATE_FAN_AUTO;
    case FanMode::FanLow:
      return CLIMATE_FAN_LOW;
    case FanMode::FanMid:
      return CLIMATE_FAN_MEDIUM;
    case FanMode::FanHigh:
      return CLIMATE_FAN_HIGH;
    default:
      return CLIMATE_FAN_AUTO;
    }
  }

  ClimateSwingMode GetSwingMode() const {
    if (!GetPowerStatus())
      return CLIMATE_SWING_OFF;
    if ((GetHorizontalSwingStatus() ==
         HorizontalSwingMode::HorizontalSwingAuto) &&
        (GetVerticalSwingStatus() == VerticalSwingMode::VerticalSwingAuto)) {
      return CLIMATE_SWING_BOTH;
    } else if (GetHorizontalSwingStatus() ==
               HorizontalSwingMode::HorizontalSwingAuto) {
      return CLIMATE_SWING_HORIZONTAL;
    } else if (GetVerticalSwingStatus() ==
               VerticalSwingMode::VerticalSwingAuto) {
      return CLIMATE_SWING_VERTICAL;
    } else {
      return CLIMATE_SWING_OFF;
    }
  }

  float GetCurrentTemperature() const {
    return status_[Offset::OffsetCurrentTemperature] / 2;
  }
  float GetTargetTemperature() const {
    return status_[Offset::OffsetSetTemperature] + 16;
  }

  bool GetFirstStatusReceived() const { return first_status_received_; }

  void LogStatus() {
    ESP_LOGD("EspHaier Status", "Readed message ALBA: %s ",
             getHex(status_).c_str());
    LogChangedBytes();
  }

  bool OnPendingData() {
    StatusMessageType data = {{255, 255}};
    if (Serial.available() == 0 || Serial.read() != 255 || Serial.read() != 255)
      return false;

    Serial.readBytes(data.data() + 2, data.size() - 2);

    if (data[Offset::OffsetCommand] != CommandType::CommandResponsePoll) {
      ESP_LOGD("EspHaier Status", "Received message is not a status: 0x%X",
               data[Offset::OffsetCommand]);
      return false;
    }

    UpdateStatus(data);

    return ValidateChecksum() && ValidateTemperature();
  }

  void SendPoll() const {
    Serial.write(poll_.data(), poll_.size());
    ESP_LOGD("EspHaier Status", "POLL: %s ", getHex(poll_).c_str());
  }

private:
  bool GetStatusDataField(uint8_t bit) const {
    return status_[Offset::OffsetStatusData] & (0x01 << bit);
  }

  bool ValidateChecksum() const {
    const byte check = getChecksum(status_);

    if (check != status_[crc_offset(status_)]) {
      ESP_LOGW("EspHaier Status", "Invalid checksum (%d vs %d)", check,
               status_[crc_offset(status_)]);
      return false;
    }
    return true;
  }

  bool ValidateTemperature() const {
    const float current_temperature = GetCurrentTemperature();
    const float target_temperature = GetTargetTemperature();

    if (current_temperature < TempConstraints::MinValidInternalTemp ||
        current_temperature > TempConstraints::MaxValidInternalTemp ||
        target_temperature < TempConstraints::MinSetTemperature ||
        target_temperature > TempConstraints::MaxSetTemperature) {
      ESP_LOGW("EspHaier Status", "Invalid temperatures");
      return false;
    }
    return true;
  }

  void UpdateStatus(const StatusMessageType &data) {
    status_ = data;

    if (GetHvacModeStatus() == AcMode::ModeFan) {
      fan_mode_fan_speed_ = GetFanSpeedStatus();
      fan_mode_setpoint_ = GetTemperatureSetpointStatus();
    } else {
      climate_mode_fan_speed_ = GetFanSpeedStatus();
      climate_mode_setpoint_ = GetTemperatureSetpointStatus();
    }
    first_status_received_ = true;
  }

  void LogChangedBytes() {
    PrintDebug();

    for (int i = 0; i < sizeof(status_); i++) {
      if (status_[i] != previous_status_[i]) {
        ESP_LOGD("EspHaier Status", "status_ byte %d: 0x%X --> 0x%X ", i,
                 previous_status_[i], status_[i]);
      }
    }

    previous_status_ = status_;
  }

  void PrintDebug() {
    if (true)
      return;
    ESP_LOGW("EspHaier Status", "Power Status = 0x%X", GetPowerStatus());
    ESP_LOGW("EspHaier Status", "HVAC return 0x%X", GetHvacModeStatus());
    ESP_LOGW("EspHaier Status", "Purify status = 0x%X", GetPurifyStatus());
    ESP_LOGW("EspHaier Status", "Quiet mode Status = 0x%X",
             GetQuietModeStatus());
    ESP_LOGW("EspHaier Status", "Fast mode Status = 0x%X", GetFastModeStatus());
    ESP_LOGW("EspHaier Status", "Fan speed Status = 0x%X", GetFanSpeedStatus());
    ESP_LOGW("EspHaier Status", "Horizontal Swing Status = 0x%X",
             GetHorizontalSwingStatus());
    ESP_LOGW("EspHaier Status", "Vertical Swing Status = 0x%X",
             GetVerticalSwingStatus());
    ESP_LOGW("EspHaier Status", "Set Point Status = 0x%X",
             GetTemperatureSetpointStatus());
  }

  byte climate_mode_fan_speed_ = FanMode::FanAuto;
  byte climate_mode_setpoint_ = 0x0A;
  byte fan_mode_fan_speed_ = FanMode::FanHigh;
  byte fan_mode_setpoint_ = 0x08;
  bool first_status_received_ = false;

  StatusMessageType status_ = GetStatusMessage();
  StatusMessageType previous_status_ = GetStatusMessage();
  PollMessageType poll_ = GetPollMessage();
};