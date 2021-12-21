#pragma once

#include "constants.h"

class Initialization {
public:
  void Initialize();

private:
  void Send(const InitializationType &initialization);

  InitializationType initialization_1 = GetInitialization1();
  InitializationType initialization_2 = GetInitialization2();
};