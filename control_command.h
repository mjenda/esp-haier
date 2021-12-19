#pragma once

#include <array>

#include "constants.h"

class ControlCommand {
public:
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

  byte* Data() { return control_command_.data(); }
  std::size_t Size() { return control_command_.size(); }

private:
  std::array<byte, 25> control_command_{
      {0xFF, 0xFF, 0x14, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x01, 0x60, 0x01, 0x09, 0x08, 0x25, 0x00, 0x02, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
};