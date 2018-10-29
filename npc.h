#ifndef NPC_H
# define NPC_H

#include <stdint.h>
#include "character.h"
#include "utils.h"

class npc : public character{
 public:
  int8_t characteristic;
  Coordinate lastKnownPosOfPC;
}

#endif
