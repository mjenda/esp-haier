#pragma once

#include <array>

#include "constants.h"
#include "utility.h"

class StatusController {
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

  bool GetQuietModeStatus() const {
    bool ret = false;
    byte tmp;
    byte msk;

    msk = (0x01 << QUIET_BIT);
    tmp = status_[STATUS_DATA_OFFSET] & msk;

    if (tmp != 0)
      ret = true;

    return ret;
  }

  bool GetPurifyStatus() const {
    bool ret = false;
    byte tmp;
    byte msk;

    msk = (0x01 << PURIFY_BIT);
    tmp = status_[STATUS_DATA_OFFSET] & msk;

    if (tmp != 0)
      ret = true;

    return ret;
  }

  bool GetPowerStatus() const {
    bool ret = false;
    byte tmp;
    byte msk;

    msk = (0x01 << POWER_BIT);
    tmp = status_[STATUS_DATA_OFFSET] & msk;

    if (tmp != 0)
      ret = true;

    return ret;
  }

  bool GetFastModeStatus() const {
    bool ret = false;
    byte tmp;
    byte msk;

    msk = (0x01 << AUTO_FAN_MAX_BIT);
    tmp = status_[STATUS_DATA_OFFSET] & msk;

    if (tmp != 0)
      ret = true;

    return ret;
  }

  float GetCurrentTemperature() const {
    return status_[TEMPERATURE_OFFSET] / 2;
  }

  ClimateMode GetMode() const {
    if (!GetPowerStatus())
      return CLIMATE_MODE_OFF;

    // Check current hvac mode
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

    // If "quiet mode" is set we will read it as "fan low"
    if (GetQuietModeStatus()) {
      return CLIMATE_FAN_LOW;
    }
    // If we detect that fast mode is on the we read it as "fan high"
    else if (GetFastModeStatus()) {
      return CLIMATE_FAN_HIGH;
    } else {
      // No quiet or fast so we read the actual fan speed.
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
  }

  ClimateSwingMode GetSwingMode() const {
    if (!GetPowerStatus())
      return CLIMATE_SWING_OFF;
    // Check the status of the swings (vertical and horizontal and translate
    // according component configuration
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

  float GetTargetTemperature() const { return status_[SET_POINT_OFFSET] + 16; }

  bool GetFirstStatusReceived() const { return first_status_received_; }

  byte GetClimateModeFanSpeed() const { return climate_mode_fan_speed_; }
  byte GetClimateModeSetpoint() const { return climate_mode_setpoint_; }
  byte GetFanModeFanSpeed() const { return fan_mode_fan_speed_; }
  byte GetFanModeSetpoint() const { return fan_mode_setpoint_; }

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
        ESP_LOGD("EspHaier StatusController", "status_ byte %d: 0x%X --> 0x%X ",
                 i, previous_status_[i], status_[i]);
      }
      previous_status_[i] = status_[i];
    }
  }

  void Update(const std::array<byte, 47> &data) {
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

  byte *Data() { return status_.data(); }
  const byte *Data() const { return status_.data(); }
  std::size_t Size() const { return status_.size(); }

  void LogStatus() {
    auto raw = getHex(Data(), Size());
    ESP_LOGD("EspHaier StatusController", "Readed message ALBA: %s ",
             raw.c_str());
    LogChangedBytes();
  }

  bool ValidateChecksum() const {
    byte check = getChecksum(Data(), Size());

    if (check != status_[CRC_OFFSET(status_)]) {
      ESP_LOGW("EspHaier StatusController", "Invalid checksum (%d vs %d)",
               check, status_[CRC_OFFSET(status_)]);
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
      ESP_LOGW("EspHaier StatusController", "Invalid temperatures");
      return false;
    }
    return true;
  }

  bool OnPendingData() {
    std::array<byte, 47> data = {{255, 255}};
    if (Serial.available() == 0 || Serial.read() != 255 || Serial.read() != 255)
      return false;

    Serial.readBytes(data.data() + 2, data.size() - 2);

    // If is a status response
    if (data[COMMAND_OFFSET] != RESPONSE_POLL) {
      ESP_LOGD("EspHaier StatusController",
               "Received message is not a status: 0x%X", data[COMMAND_OFFSET]);
      return false;
    }

    Update(data);

    return ValidateChecksum() && ValidateTemperature();
  }

  void SendPoll() const {
    Serial.write(poll_.data(), poll_.size());
    const auto raw = getHex(poll_.data(), poll_.size());
    ESP_LOGD("EspHaier StatusController", "POLL: %s ", raw.c_str());
  }

  void PrintDebug() {
    if (true)
      return;
    ESP_LOGW("EspHaier StatusController", "Power StatusController = 0x%X",
             GetPowerStatus());
    ESP_LOGW("EspHaier StatusController", "HVAC return 0x%X",
             GetHvacModeStatus());
    ESP_LOGW("EspHaier StatusController", "Purify status = 0x%X",
             GetPurifyStatus());
    ESP_LOGW("EspHaier StatusController", "Quiet mode StatusController = 0x%X",
             GetQuietModeStatus());
    ESP_LOGW("EspHaier StatusController", "Fast mode StatusController = 0x%X",
             GetFastModeStatus());
    ESP_LOGW("EspHaier StatusController", "Fan speed StatusController = 0x%X",
             GetFanSpeedStatus());
    ESP_LOGW("EspHaier StatusController",
             "Horizontal Swing StatusController = 0x%X",
             GetHorizontalSwingStatus());
    ESP_LOGW("EspHaier StatusController",
             "Vertical Swing StatusController = 0x%X",
             GetVerticalSwingStatus());
    ESP_LOGW("EspHaier StatusController", "Set Point StatusController = 0x%X",
             GetTemperatureSetpointStatus());
  }

private:
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