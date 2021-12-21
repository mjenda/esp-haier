#pragma once

#include <array>

enum Offset {
  OffsetCommand = 9,
  OffsetSetTemperature = 12,
  OffsetVerticalSwing = 13,
  OffsetMode = 14,
  OffsetStatusData = 17,
  OffsetHorizontalSwing = 19,
  OffsetCurrentTemperature = 22,
  OffsetSwing = 27,
  OffsetLock = 28,
  OffsetFresh = 31,
};

enum AcMode {
  ModeMask = 0xF0,
  ModeAuto = 0x00,
  ModeDry = 0x40,
  ModeCool = 0x20,
  ModeHeat = 0x80,
  ModeFan = 0xC0,
};

enum FanMode {
  FanMask = 0x0F,
  FanLow = 0x03,
  FanMid = 0x02,
  FanHigh = 0x01,
  FanAuto = 0x05,
};

enum HorizontalSwingMode {
  HorizontalSwingCenter = 0x00,
  HorizontalSwingMaxLeft = 0x03,
  HorizontalSwingLeft = 0x04,
  HorizontalSwingMaxRight = 0x06,
  HorizontalSwingRight = 0x05,
  HorizontalSwingAuto = 0x07,
};

enum VerticalSwingMode {
  VerticalSwingMaxUp = 0x02,
  VerticalSwingUp = 0x04,
  VerticalSwingCenter = 0x06,
  VerticalSwingDown = 0x08,
  VerticalSwingHealthUp = 0x01,
  VerticalSwingHealthDown = 0x03,
  VerticalSwingAuto = 0x0C,
};

enum DataField {
  DataFieldPower = 0,
  DataFieldPurify = 0x01,
  DataFieldQuiet = 0x03,
  DataFieldFanMax = 0x04,
};

enum SwingMode {
  SwingOff = 0x00,
  SwingVertical = 0x01,
  SwingHorizontal = 0x02,
  SwingBoth = 0x03,
};

enum LockState {
  LockStateOn = 0x50,
  LockStateOff = 0x00,
};

enum FreshState {
  FreshkStateOn = 0x01,
  FreshStateOff = 0x00,
};

enum CommandType {
  CommandResponsePoll = 0x02,
};

enum PowerControl {
  PowerControllOffset = 13,
  PowerControlOn = 0x01,
  PwoerControlOff = 0x00,
};

enum TempConstraints {
  MinSetTemperature = 16,
  MaxSetTemperature = 30,
  MinValidInternalTemp = 10,
  MaxValidInternalTemp = 50,
};

constexpr auto GetPollMessage = []() {
  return std::array<byte, 15>({0xFF, 0xFF, 0x0A, 0x40, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x01, 0x4D, 0x01, 0x99, 0xB3, 0xB4});
};

constexpr auto GetControlMessage = []() {
  return std::array<byte, 25>({0xFF, 0xFF, 0x14, 0x40, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x01, 0x60, 0x01, 0x09, 0x08,
                               0x25, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00});
};

constexpr auto GetStatusMessage = []() { return std::array<byte, 47>(); };

constexpr auto GetInitialization1 = []() {
  return std::array<byte, 13>({0xFF, 0xFF, 0x0A, 0x0, 0x0, 0x0, 0x0, 0x0, 0x00,
                               0x61, 0x00, 0x07, 0x72});
};

constexpr auto GetInitialization2 = []() {
  return std::array<byte, 13>({0xFF, 0xFF, 0x08, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0,
                               0x70, 0xB8, 0x86, 0x41});
};

using PollMessageType = decltype(GetPollMessage());
using ControlMessagType = decltype(GetControlMessage());
using StatusMessageType = decltype(GetStatusMessage());
using InitializationType = decltype(GetInitialization1());