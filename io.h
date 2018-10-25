#ifndef IO_H
# define IO_H

#include "dungeonInfo.h"

void initTerminal();
void endTerminal();
void displayMonsterList(Dungeon *d);
void handleUserInput(Dungeon *d);
void teleportPlayer(Dungeon *d);
void moveLevel(Dungeon *d);
void newPrintMap(Dungeon *d);
void printFog(Dungeon *d);
bool withinSight(Dungeon *d, uint8_t row, uint8_t col);

#endif
