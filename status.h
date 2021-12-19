#pragma once

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

  float GetCurrentTemperature() const { return status_[TEMPERATURE_OFFSET] / 2; }

  float GetTargetTemperature() const { return status_[SET_POINT_OFFSET] + 16; }

  void CompareStatusByte() {
    int i;

    if (previous_status_init == false) {
      for (i = 0; i < sizeof(status_); i++) {
        previous_status[i] = status_[i];
      }
      previous_status_init = true;
    }

    for (i = 0; i < sizeof(status_); i++) {
      if (status_[i] != previous_status[i]) {
        ESP_LOGD("Debug", "status_ byte %d: 0x%X --> 0x%X ", i,
                 previous_status[i], status_[i]);
      }
      previous_status[i] = status_[i];
    }
  }

  void Update(const std::array<byte, 47> &data) {
    status_ = data;
  }

  byte *Data() { return status_.data(); }
  const byte *Data() const { return status_.data(); }
  std::size_t Size() const { return status_.size(); }

  void EspLog() const {
    auto raw = getHex(Data(), Size());
    ESP_LOGD("Haier", "Readed message ALBA: %s ", raw.c_str());
  }

  bool ValidateChecksum() const {
    byte check = getChecksum(Data(), Size());

    if (check != status_[CRC_OFFSET(status_)]) {
      ESP_LOGW("Haier", "Invalid checksum (%d vs %d)", check,
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
      ESP_LOGW("Haier", "Invalid temperatures");
      return false;
    }
    return true;
  }

private:
  bool previous_status_init = false;

  std::array<byte, 47> status_{{}};
  std::array<byte, 47> previous_status{{}};
};