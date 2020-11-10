#ifndef DUNGEONINFO_H
# define DUNGEONINFO_H

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <cstdlib>

#include "heap.h"
#include "utils.h"

#define DUNGEON_WIDTH  80
#define DUNGEON_HEIGHT 21
#define MIN_ROOM_WIDTH 3
#define MAX_ROOM_WIDTH 10
#define MIN_ROOM_HEIGHT 2
#define MAX_ROOM_HEIGHT 10
#define MIN_NUMBER_OF_ROOMS 5
#define MAX_NUMBER_OF_ROOMS  10
#define MIN_HARDNESS 0
#define MAX_HARDNESS 255

#define SUCCESS 0
#define FAILURE 1

#define NPC_SMART 0x00000001
#define NPC_TELEPATH 0x00000002
#define NPC_TUNNEL 0x00000004
#define NPC_ERRATIC 0x00000008

/* Room with points one and two. width = 5, height = 4
  1 - - - -
  - - - - -
  - - - - -
  - - - - -
 */
typedef struct{
  Coordinate topLeftCoord;
  uint8_t width;
  uint8_t height;
  int isConnected;
}Room;

typedef struct{
  heap_node_t *hn;
  Coordinate location;
  int alive;
  int8_t characteristic;
  Coordinate lastKnownPosOfPC;
  int8_t speed;
  int turn;
  char type;
  char seenMap[DUNGEON_HEIGHT][DUNGEON_WIDTH];
}Character;


typedef struct{
  std::string name;
  std::string desc;
  std::string color;
  std::string speed;
  std::string abil;
  std::string hp;
  std::string dam;
  std::string symb;
  std::string rrty;
  int count;
}monsterDesc;

//monster_desc.txt
enum monsterFileType{
  BEGIN,
  NAME,
  DESC,
  COLOR,
  SPEED,
  ABIL,
  HP,
  DAM,
  SYMB,
  RRTY,
  END,
  UNKNOWN
};

typedef struct{
  uint32_t numOfRooms;
  Room *rooms;
  char map[DUNGEON_HEIGHT][DUNGEON_WIDTH];
  uint32_t numOfMonsters;
  Character *monsters;
  uint8_t hardnessMap[DUNGEON_HEIGHT][DUNGEON_WIDTH];
  int8_t nonTunnelPaths[DUNGEON_HEIGHT][DUNGEON_WIDTH];
  int8_t tunnelPaths[DUNGEON_HEIGHT][DUNGEON_WIDTH];
  char characterMap[DUNGEON_HEIGHT][DUNGEON_WIDTH];
  Character pc;
  int runs;
  bool fogOfWar;
  std::vector<monsterDesc> monsterDescriptions;
}Dungeon;

typedef struct{
  heap_node_t *hn;
  uint8_t pos[2];
  Coordinate coord;
  int32_t cost;
}dist_t;

typedef enum dim {
  dim_x,
  dim_y,
  num_dims
} dim_t;

/*
class monsterDesc{
 public:
  std::string name;
  std::string desc;
  std::string color;
  int8_t speed;
  std::string abil;
  int32_t hp;
  int32_t dam;
  char symb;
  int32_t rrty;
  int count;
  void setName(std::string inputName);
  std::string getName() {return name;}
  void setDesc(std::string inputDesc);
  std::string getDesc() {return desc;}
  void setColor(std::string inputColor);
  std::string getColor(){return color;}
  void setSpeed(int8_t speed);
  int8_t getSpeed(){return speed;}
  void setAbil(std::string inputAbil);
  std::string getAbil(){return abil;}
  void setHp(int32_t inputHP);
  int32_t getHP(){return hp;}
  void setDam(int32_t inputDam);
  int32_t getDam(){return dam;}
  void setSymb(char inputSymb);
  char getSymb(){return dam;}
  void setRrty(int32_t inputRrty);
  int32_t getRrty(){return rrty;}
  void incrementCount();
};

void monsterDesc::setName(std::string inputName){}
void monsterDesc::setDesc(std::string inputDesc){}
void monsterDesc::setColor(std::string inputColor){}
void monsterDesc::setSpeed(int8_t inputSpeed){}

*/



#endif
