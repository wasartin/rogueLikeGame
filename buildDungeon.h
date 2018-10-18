#ifndef BUILDDUNGEON_H
# define BUILDDUNGEON_H

#include "dungeonInfo.h"

void initlizeDungeon(Dungeon *d);
int isLegalPlace(int row, int col, Dungeon *d, Room *r);
void generateRooms();
void placeRooms();
void setSortedRoomArray(Dungeon *d);
int generateRange(int min, int max);
void connectRooms(Dungeon *d);
void fillMap(Dungeon *d, int RoomNumber);
void generateStairs(Dungeon *d, int row, int col, int dir);

#endif
