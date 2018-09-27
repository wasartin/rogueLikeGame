#ifndef DUNGEONINFO_H
# define DUNGEONINFO_H

#include <stdio.h>
#include <stdint.h>

#include "heap.h"


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

#define TRUE  0
#define FALSE 1 

typedef struct{
  uint8_t row;
  uint8_t col;
}Coordinate;

/* Room with points one and two. width = 5, height = 4
  1 - - - -
  - - - - -
  - - - - -
  - - - - 2
 */
typedef struct{
  Coordinate topLeftCoord;
  uint8_t width;
  uint8_t height;
  int isConnected;
}Room;

typedef struct{
  Coordinate location;
}Player;

typedef struct{
  uint32_t numOfRooms;
  Room *rooms;
  char map[DUNGEON_HEIGHT][DUNGEON_WIDTH];
  uint8_t hardnessMap[DUNGEON_HEIGHT][DUNGEON_WIDTH];
  int8_t nonTunnelPaths[DUNGEON_HEIGHT][DUNGEON_WIDTH];
  int8_t tunnelPaths[DUNGEON_HEIGHT][DUNGEON_WIDTH];
  Player pc;
} Dungeon;

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
