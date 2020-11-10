#include <stdio.h>
#include <stdlib.h>

#include "character.h"
#include "buildDungeon.h"
#include "dungeonInfo.h"

void makeMonsters(Dungeon *d){
  uint8_t i;
  for(i = 0; i < d->numOfMonsters; i++){
    d->monsters[i].characteristic = rand() & 0xf;
    d->monsters[i].speed = generateRange(5, 21);
    d->monsters[i].alive = true;
    d->monsters[i].lastKnownPosOfPC.row = 0;
    d->monsters[i].lastKnownPosOfPC.col = 0;
    d->monsters[i].turn = 1000 / d->monsters[i].speed;
    switch(d->monsters[i].characteristic){
    case 0x0:
      d->monsters[i].type = '0';
      break;
    case 0x1:
      d->monsters[i].type = '1';
      break;
    case 0x2:
      d->monsters[i].type = '2';
      break;
    case 0x3:
      d->monsters[i].type = '3';
      break;
    case 0x4:
      d->monsters[i].type = '4';
      break;
    case 0x5:
      d->monsters[i].type = '5';
      break;
    case 0x6:
      d->monsters[i].type = '6';
      break;
    case 0x7:
      d->monsters[i].type = '7';
      break;
    case 0x8:
      d->monsters[i].type = '8';
      break;
    case 0x9:
      d->monsters[i].type = '9';
      break;
    case 0xa:
      d->monsters[i].type = 'a';
      break;
    case 0xb:
      d->monsters[i].type = 'b';
      break;
    case 0xc:
      d->monsters[i].type = 'c';
      break;
    case 0xd:
      d->monsters[i].type = 'd';
      break;
    case 0xe:
      d->monsters[i].type = 'e';
      break;
    case 0xf:
      d->monsters[i].type = 'f';
      break;
    default:
      break;
    } 
  }
}

void placeMonsters(Dungeon *d){
  uint8_t i;
  for(i = 0; i < d->numOfMonsters; i++){
    int placed = false;
    while(placed == false){
      int randomRoom = generateRange(1, d->numOfRooms);
      Room currRoom = d->rooms[randomRoom];
      int randomRow = generateRange(0, currRoom.height);
      int randomCol = generateRange(0, currRoom.width);
      int selectedRow = currRoom.topLeftCoord.row + randomRow;
      int selectedCol = currRoom.topLeftCoord.col + randomCol;
      if(d->hardnessMap[selectedRow][selectedCol] != 255){
	if(d->characterMap[selectedRow][selectedCol] == '\0'){
	  d->characterMap[selectedRow][selectedCol] = d->monsters[i].type;
	  d->monsters[i].location.row = selectedRow;
	  d->monsters[i].location.col = selectedCol;
	  placed = true;
	}
      } 
    }
  }
}

