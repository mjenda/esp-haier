#pragma once

#include "esphome.h"
#include <array>

#include "constants.h"
#include "utility.h"


class Status {
public:
  byte GetHvacModeStatus() const { return status_[MODE_OFFSET] & MODE_MSK; }

  byte GetTemperatureSetpointStatus() const {
    return status_[SET_POINT_OFFSET];
  }

  byte GetFanSpeedStatus() const { return status_[MODE_OFFSET] & FAN_MSK; }

  byte GetHorizontalSwingStatus() const {
    return status_[HORIZONTAL_SWING_OFFSET];
  }

  byte GetVerticalSwingStatus() const { return status_[VERTICAL_SWING_OFFSET]; }

  byte GetClimateModeFanSpeed() const { return climate_mode_fan_speed_; }

  byte GetClimateModeSetpoint() const { return climate_mode_setpoint_; }

  byte GetFanModeFanSpeed() const { return fan_mode_fan_speed_; }

  byte GetFanModeSetpoint() const { return fan_mode_setpoint_; }

  bool GetPurifyStatus() const { return GetStatusDataField(PURIFY_BIT); }

  bool GetPowerStatus() const { return GetStatusDataField(POWER_BIT); }

  bool GetQuietModeStatus() const { return GetStatusDataField(QUIET_BIT); }

  bool GetFastModeStatus() const {
    return GetStatusDataField(AUTO_FAN_MAX_BIT);
  }

  ClimateMode GetMode() const {
    if (!GetPowerStatus())
      return CLIMATE_MODE_OFF;

    switch (GetHvacModeStatus()) {
    case MODE_COOL:
      return CLIMATE_MODE_COOL;
      break;
    case MODE_HEAT:
      return CLIMATE_MODE_HEAT;
      break;
    case MODE_DRY:
      return CLIMATE_MODE_DRY;
      break;
    case MODE_FAN:
      return CLIMATE_MODE_FAN_ONLY;
      break;
    case MODE_AUTO:
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
    case FAN_AUTO:
      return CLIMATE_FAN_AUTO;
      break;
    case FAN_LOW:
      return CLIMATE_FAN_LOW;
      break;
    case FAN_MID:
      return CLIMATE_FAN_MEDIUM;
      break;
    case FAN_HIGH:
      return CLIMATE_FAN_HIGH;
      break;
    default:
      return CLIMATE_FAN_AUTO;
    }
  }

  ClimateSwingMode GetSwingMode() const {
    if (!GetPowerStatus())
      return CLIMATE_SWING_OFF;
    if ((GetHorizontalSwingStatus() == HORIZONTAL_SWING_AUTO) &&
        (GetVerticalSwingStatus() == VERTICAL_SWING_AUTO)) {
      return CLIMATE_SWING_BOTH;
    } else if (GetHorizontalSwingStatus() == HORIZONTAL_SWING_AUTO) {
      return CLIMATE_SWING_HORIZONTAL;
    } else if (GetVerticalSwingStatus() == VERTICAL_SWING_AUTO) {
      return CLIMATE_SWING_VERTICAL;
    } else {
      return CLIMATE_SWING_OFF;
    }
  }

  float GetCurrentTemperature() const {
    return status_[TEMPERATURE_OFFSET] / 2;
  }
  float GetTargetTemperature() const { return status_[SET_POINT_OFFSET] + 16; }

  bool GetFirstStatusReceived() const { return first_status_received_; }

  void LogStatus() {
    auto raw = getHex(status_.data(), status_.size());
    ESP_LOGD("EspHaier Status", "Readed message ALBA: %s ", raw.c_str());
    LogChangedBytes();
  }

  bool OnPendingData() {
    std::array<byte, 47> data = {{255, 255}};
    if (Serial.available() == 0 || Serial.read() != 255 || Serial.read() != 255)
      return false;

    Serial.readBytes(data.data() + 2, data.size() - 2);

    if (data[COMMAND_OFFSET] != RESPONSE_POLL) {
      ESP_LOGD("EspHaier Status", "Received message is not a status: 0x%X",
               data[COMMAND_OFFSET]);
      return false;
    }

    UpdateStatus(data);

    return ValidateChecksum() && ValidateTemperature();
  }

  void SendPoll() const {
    Serial.write(poll_.data(), poll_.size());
    const auto raw = getHex(poll_.data(), poll_.size());
    ESP_LOGD("EspHaier Status", "POLL: %s ", raw.c_str());
  }

private:
  bool GetStatusDataField(uint8_t bit) const {
    return status_[STATUS_DATA_OFFSET] & (0x01 << bit);
  }

  bool ValidateChecksum() const {
    const byte check = getChecksum(status_.data(), status_.size());

    if (check != status_[CRC_OFFSET(status_)]) {
      ESP_LOGW("EspHaier Status", "Invalid checksum (%d vs %d)", check,
               status_[CRC_OFFSET(status_)]);
      return false;
    }
    return true;
  }

  bool ValidateTemperature() const {
    const float current_temperature = GetCurrentTemperature();
    const float target_temperature = GetTargetTemperature();

    if (current_temperature < MIN_VALID_INTERNAL_TEMP ||
        current_temperature > MAX_VALID_INTERNAL_TEMP ||
        target_temperature < MIN_SET_TEMPERATURE ||
        target_temperature > MAX_SET_TEMPERATURE) {
      ESP_LOGW("EspHaier Status", "Invalid temperatures");
      return false;
    }
    return true;
  }

  void UpdateStatus(const std::array<byte, 47> &data) {
    status_ = data;

    if (GetHvacModeStatus() == MODE_FAN) {
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

    int i;

    if (!previous_status_init_) {
      for (i = 0; i < sizeof(status_); i++) {
        previous_status_[i] = status_[i];
      }
      previous_status_init_ = true;
    }

    for (i = 0; i < sizeof(status_); i++) {
      if (status_[i] != previous_status_[i]) {
        ESP_LOGD("EspHaier Status", "status_ byte %d: 0x%X --> 0x%X ", i,
                 previous_status_[i], status_[i]);
      }
      previous_status_[i] = status_[i];
    }
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

  bool previous_status_init_ = false;
  byte climate_mode_fan_speed_ = FAN_AUTO;
  byte climate_mode_setpoint_ = 0x0A;
  byte fan_mode_fan_speed_ = FAN_HIGH;
  byte fan_mode_setpoint_ = 0x08;
  bool first_status_received_ = false;

  std::array<byte, 47> status_{{}};
  std::array<byte, 47> previous_status_{{}};
  std::array<byte, 15> poll_{{0xFF, 0xFF, 0x0A, 0x40, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x01, 0x4D, 0x01, 0x99, 0xB3, 0xB4}};
};