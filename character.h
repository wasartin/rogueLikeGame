#ifndef CHARACTER_H
# define CHARACTER_H

#include "dungeonInfo.h"
#include "heap.h"


/*
class character{
 public:
  heap_node_t *hn;
  Coordinate location;
  int alive;
  int8_t characteristic;
  Coordinate lastKnownPosOfPC;
  int8_t speed;
  int turn;
  char type;
}
*/
void makeMonsters(Dungeon *d);
void placeMonsters(Dungeon *d);
int isInSameRoom(Dungeon *d, Character *monster);
int proximityCheck(Dungeon *d, Character *monster);
void moveMonster(Dungeon *d, Character *monster);
void moveCharacter(Dungeon *d, int row, int col, Character *curr);
void sortDeadMonsters(Dungeon *d);

bool checkTargetCoordinate(Dungeon *d, uint8_t row, uint8_t col);

//getters and setters
/*
Coordinate getLocation();
void setLocation();
int isAlive();
void setAlive();
void setCharacteristic();
int8_t getCharacteristic();
void setSpeed();
int8_t getSpeed();
void setType();
char getType();
*/

#endif
