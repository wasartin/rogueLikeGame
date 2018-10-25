#ifndef MOVE_H
#define MOVE_H

#include "dungeonInfo.h"

void moveCharacter(Dungeon *d, int targetRow, int targetCol, Character *curr);
int proximityCheck(Dungeon *d, Character *monster);
void moveMonster(Dungeon *d, Character *monster);
void sortDeadMonsters(Dungeon *d);
bool checkTargetCoordinateTeleport(Dungeon *d, uint8_t row, uint8_t col);
bool checkTargetCoordinate(Dungeon *d, uint8_t row, uint8_t col);


#endif
