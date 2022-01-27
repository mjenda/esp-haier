#pragma once

#include "esphome.h"

#include "status.h"
class Haier : public esphome::climate::Climate,
              public esphome::PollingComponent {
public:
  Haier();

  // Climate overrides
  void setup() override;
  void loop() override;
  void control(const esphome::climate::ClimateCall &call) override;

  // PollingComponent overrides
  void update() override;

protected:
  esphome::climate::ClimateTraits traits() override;

private:
  Status status_;
};
