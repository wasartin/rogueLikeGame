#ifndef CHARACTER_H
# define CHARACTER_H

#include "dungeonInfo.h"

void makeMonsters(Dungeon *d);
void placeMonsters(Dungeon *d);
int isInSameRoom(Dungeon *d, Character *monster);
int proximityCheck(Dungeon *d, Character *monster);
void moveMonster(Dungeon *d, Character *monster);
void moveCharacter(Dungeon *d, int row, int col, Character *curr);
void sortDeadMonsters(Dungeon *d);

#endif
