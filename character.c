#include <stdio.h>
#include <stdlib.h>

#include "character.h"
#include "pathFinding.h"
#include "buildDungeon.h"
#include "dungeonInfo.h"

void makeMonsters(Dungeon *d){
  int i;
  for(i = 0; i < d->numOfMonsters; i++){
    /*
    if(i % 2 == 0){
      d->monsters[i].characteristic = 0x3;
    }else{
      d->monsters[i].characteristic = 0x7;
    }
    */
    d->monsters[i].characteristic = rand() & 0xf;
    d->monsters[i].speed = generateRange(5, 21);
    d->monsters[i].alive = SUCCESS;
    d->monsters[i].lastKnownPosOfPC.row = 0;
    d->monsters[i].lastKnownPosOfPC.col = 0;
    d->monsters[i].turn = 1000 / d->monsters[i].speed;
    switch(d->monsters[i].characteristic){
    case 0x0:
      d->monsters[i].representation = '0';
      break;
    case 0x1:
      d->monsters[i].representation = '1';
      break;
    case 0x2:
      d->monsters[i].representation = '2';
      break;
    case 0x3:
      d->monsters[i].representation = '3';
      break;
    case 0x4:
      d->monsters[i].representation = '4';
      break;
    case 0x5:
      d->monsters[i].representation = '5';
      break;
    case 0x6:
      d->monsters[i].representation = '6';
      break;
    case 0x7:
      d->monsters[i].representation = '7';
      break;
    case 0x8:
      d->monsters[i].representation = '8';
      break;
    case 0x9:
      d->monsters[i].representation = '9';
      break;
    case 0xa:
      d->monsters[i].representation = 'a';
      break;
    case 0xb:
      d->monsters[i].representation = 'b';
      break;
    case 0xc:
      d->monsters[i].representation = 'c';
      break;
    case 0xd:
      d->monsters[i].representation = 'd';
      break;
    case 0xe:
      d->monsters[i].representation = 'e';
      break;
    case 0xf:
      d->monsters[i].representation = 'f';
      break;
    default:
      break;
    } 
  }
}

void placeMonsters(Dungeon *d){
  int i;
  for(i = 0; i < d->numOfMonsters; i++){
    int placed = 0;
    while(!placed){
      int randomRoom = generateRange(1, d->numOfRooms);
      Room currRoom = d->rooms[randomRoom];
      int randomRow = generateRange(0, currRoom.height);
      int randomCol = generateRange(0, currRoom.width);
      int selectedRow = currRoom.topLeftCoord.row + randomRow;
      int selectedCol = currRoom.topLeftCoord.col + randomCol;
      if(d->hardnessMap[selectedRow][selectedCol] != 255){
	if(d->characterMap[selectedRow][selectedCol] == '\0'){
	  d->characterMap[selectedRow][selectedCol] = d->monsters[i].representation;
	  d->monsters[i].location.row = selectedRow;
	  d->monsters[i].location.col = selectedCol;
	  placed = 1;
	}
      } 
    }
  }
}

void moveMonster(Dungeon *d, Character *monster){
  int currRow = monster->location.row;
  int currCol = monster->location.col;
  int targetRow, targetCol;
  int chance = rand() % 2;
  if((monster->characteristic & NPC_TELEPATH) || (proximityCheck(d, monster) == SUCCESS)){
    monster->lastKnownPosOfPC.row = d->pc.location.row;
    monster->lastKnownPosOfPC.col = d->pc.location.col;
  }

  Coordinate neighbors[9];
  int i, j, neighborLength;
  neighborLength = 0;
  for(i = -1; i < 2; i++){
    for(j = -1; j < 2; j++){
      //if  within bounds, throw it into the neighbors
      if((currRow + i > 0 && currRow + i < 20) && (currCol + j > 0 && currCol + j < 79)){
	if((monster->characteristic & NPC_TUNNEL) && d->hardnessMap[currRow + i][currCol + j] != 255){
	  //allow all round
	  neighbors[neighborLength].row = currRow + i;
	  neighbors[neighborLength].col = currCol + j;
	  neighborLength++;
	}else{
	  if(d->hardnessMap[currRow + i][currCol + j] == MIN_HARDNESS){
	    neighbors[neighborLength].row = currRow + i;
	    neighbors[neighborLength].col = currCol + j;
	    neighborLength++;
	  }
	} 
      }
    }
  }

  if((monster->characteristic & NPC_ERRATIC) && (chance == 0)){
    chance = 0;
  }else {
    chance = 1;
  }

  if((monster->characteristic & NPC_SMART) && (monster->lastKnownPosOfPC.row != 0)){
    int smallestValue, tmpValue;
    if(monster->characteristic & NPC_TUNNEL){
      int smallestValue =  85;
      for(i = 0; i < neighborLength; i++){
	tmpValue = d->tunnelPaths[neighbors[i].row][neighbors[i].col];
	if(tmpValue < smallestValue){
	  smallestValue = tmpValue;
	  targetRow = neighbors[i].row;
	  targetCol = neighbors[i].col;
	}
      }
    }else {
      smallestValue = 85;
      for(i = 0; i < neighborLength; i++){
	tmpValue = d->nonTunnelPaths[neighbors[i].row][neighbors[i].col];
	if(tmpValue < smallestValue){
	  smallestValue = tmpValue;
	  targetRow = neighbors[i].row;
	  targetCol = neighbors[i].col;
	}
      }
    }
  }
  else if(monster->lastKnownPosOfPC.row != 0){ //does see player
    int smallestRowDiff = monster->lastKnownPosOfPC.row - monster->location.row;
    int smallestColDiff = monster->lastKnownPosOfPC.col - monster->location.col;
    targetRow = monster->location.row;
    targetCol = monster->location.col;
    int i;
    int rowPriority = rand() % 2;
    if(rowPriority == 0 && smallestRowDiff != 0){
      for(i = 0; i < neighborLength; i++){
	if(monster->lastKnownPosOfPC.row - neighbors[i].row < smallestRowDiff){
	  smallestRowDiff = neighbors[i].row;
	  targetRow = neighbors[i].row;
	  targetCol = neighbors[i].col;
	}
      }
    }else {
      for(i = 0; i < neighborLength; i++){
	if(monster->lastKnownPosOfPC.col - neighbors[i].col < smallestColDiff){
	  smallestColDiff = neighbors[i].col;
	  targetRow = neighbors[i].row;
	  targetCol = neighbors[i].col;
	}
      }
    }
  }
  //Add in case if the player is seen/
  else {
    chance = 0;
  }
  if(d->runs > 10 && !(monster->characteristic & NPC_TELEPATH)){
    chance = 0;
  }
  if(chance == 0){
    //move erratic inside of neighbors
    int num = generateRange(0, neighborLength);
    targetRow = neighbors[num].row;
    targetCol = neighbors[num].col;
    
  }
  moveCharacter(d, targetRow, targetCol, monster);
}

int isInSameRoom(Dungeon *d, Character *monster){
  int i;
  for(i = 0; i < d->numOfRooms; i++){
    int playerHere = 1;
    int monsterHere = 1;
    int currRow;
    int currCol;
    for(currRow = d->rooms[i].topLeftCoord.row; currRow < d->rooms[i].topLeftCoord.row +
	  d->rooms[i].height; currRow++){
      for(currCol = d->rooms[i].topLeftCoord.col; currCol < d->rooms[i].topLeftCoord.row +
	    d->rooms[i].width; currCol++){
	if(currRow == d->pc.location.row && currCol == d->pc.location.col){
	  playerHere = 0;
	}
	if(currRow == monster->location.row && 
	   currCol == monster->location.col){
	  monsterHere = 0;
	}
      }
    }
    if(playerHere == 0 && monsterHere == 0){
      return SUCCESS;
    }else {
      playerHere = monsterHere = FAILURE;
    }
  }
  return FAILURE;
}

int proximityCheck(Dungeon *d, Character *monster){
  int rowDifference = d->pc.location.row - monster->location.row;
  int colDifference = d->pc.location.col - monster->location.col;
  if((rowDifference <= 5 && rowDifference >= -5) &&
     (colDifference <= 5 && colDifference >= -5)){
    return 0;
  }
  return 1;
}

void moveCharacter(Dungeon *d, int targetRow, int targetCol, Character *curr){
  if(curr->alive == 0){
    if(d->hardnessMap[curr->location.row][curr->location.col] != MIN_HARDNESS){
      d->map[curr->location.row][curr->location.col] = '#';
      d->hardnessMap[curr->location.row][curr->location.col] = 0;
    }
    if(d->characterMap[targetRow][targetCol] != '\0'){
      if(curr->representation != '@'){
	if(d->pc.location.row == targetRow && d->pc.location.col == targetCol){
	  d->pc.alive = 1;
	}else{
	  int i;
	  for(i = 0; i < d->numOfMonsters; i++){
	    if(d->monsters[i].location.row == targetRow && d->monsters[i].location.col == targetCol){
	      d->monsters[i].alive = 1;
	      //might cut this out
	      //d->numOfMonsters--;
	    }
	  }
	}
      }
      d->characterMap[targetRow][targetCol] = '\0';
    }
    d->characterMap[curr->location.row][curr->location.col] = '\0';
    d->characterMap[targetRow][targetCol] = curr->representation;
    curr->location.row = targetRow;
    curr->location.col = targetCol;
  }
}
