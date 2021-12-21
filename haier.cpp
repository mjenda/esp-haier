#include "haier.h"

#include "esphome.h"

#include "control.h"
#include "constants.h"
#include "initialization.h"

using esphome::esp_log_printf_;
using esphome::climate::ClimateCall;
using esphome::climate::ClimateMode;
using esphome::climate::ClimateFanMode;
using esphome::climate::ClimateSwingMode;
using esphome::climate::ClimateTraits;

Haier::Haier() : PollingComponent(kPollingIntervalInMilisec) {}

void Haier::setup() {
  Serial.begin(9600);
  Initialization().Initialize();
}

void Haier::loop() {
  if (!status_.OnPendingData())
    return;

  status_.LogStatus();

  Climate::mode = status_.GetMode();
  Climate::fan_mode = status_.GetFanMode();
  Climate::swing_mode = status_.GetSwingMode();
  Climate::current_temperature = status_.GetCurrentTemperature();
  Climate::target_temperature = status_.GetTargetTemperature();
  Climate::publish_state();
}

void Haier::update() { status_.SendPoll(); }

void Haier::control(const ClimateCall &call) {
  ESP_LOGD("EspHaier Control", "Control call");

  if (!status_.GetFirstStatusReceived()) {
    ESP_LOGD("EspHaier Control", "No action, first poll answer not received");
    return;
  }

  Control(status_, call).Send();
}

ClimateTraits Haier::traits() {
  auto traits = esphome::climate::ClimateTraits();
  traits.set_supported_modes(
      {ClimateMode::CLIMATE_MODE_OFF, ClimateMode::CLIMATE_MODE_HEAT_COOL,
       ClimateMode::CLIMATE_MODE_HEAT, ClimateMode::CLIMATE_MODE_COOL,
       ClimateMode::CLIMATE_MODE_DRY, ClimateMode::CLIMATE_MODE_FAN_ONLY});

  traits.set_supported_fan_modes(
      {ClimateFanMode::CLIMATE_FAN_AUTO, ClimateFanMode::CLIMATE_FAN_LOW,
       ClimateFanMode::CLIMATE_FAN_MEDIUM, ClimateFanMode::CLIMATE_FAN_HIGH});

  traits.set_visual_min_temperature(TempConstraints::MinSetTemperature);
  traits.set_visual_max_temperature(TempConstraints::MaxSetTemperature);
  traits.set_visual_temperature_step(1.0f);
  traits.set_supports_current_temperature(true);

  traits.set_supported_swing_modes(
      {ClimateSwingMode::CLIMATE_SWING_OFF,
       ClimateSwingMode::CLIMATE_SWING_BOTH,
       ClimateSwingMode::CLIMATE_SWING_VERTICAL,
       ClimateSwingMode::CLIMATE_SWING_HORIZONTAL});
  return traits;
}