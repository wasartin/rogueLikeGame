#ifndef DUNGEONINFO_H
# define DUNGEONINFO_H

#include <stdio.h>
#include <stdint.h>

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


class ;

typedef struct{
 public:
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

#endif
