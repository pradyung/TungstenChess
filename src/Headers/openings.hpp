#pragma once

#include <stdint.h>
#include "move.hpp"

namespace Chess
{
  class Openings
  {
  public:
    static Move *getOpeningMoves();

    static const uint16_t openings[];
  };
}