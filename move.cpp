#include <cstdlib>

#include "move.h"
#include "dungeonInfo.h"
#include "buildDungeon.h"

void moveMonster(Dungeon *d, Character *monster){
  int currRow = monster->location.row;
  int currCol = monster->location.col;
  int targetRow, targetCol;
  int chance = rand() % 2;
  if((monster->characteristic & NPC_TELEPATH) || (proximityCheck(d, monster) == 0)){
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
	if((monster->characteristic & NPC_TUNNEL) && d->hardnessMap[currRow + i][currCol + j] != MAX_HARDNESS){
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
    int smallestRowDiff = abs(monster->lastKnownPosOfPC.row - monster->location.row);
    int smallestColDiff = abs(monster->lastKnownPosOfPC.col - monster->location.col);
    Coordinate targetCoord;
    targetCoord.row = monster->location.row;
    targetCoord.col = monster->location.col;
    int i;
    //find the smallest Row and Col Diff
    for(i = 0; i < neighborLength; i++){
      int newRowDiff = abs(monster->lastKnownPosOfPC.row - neighbors[i].row);
      int newColDiff = abs(monster->lastKnownPosOfPC.col - neighbors[i].col);
      if((smallestRowDiff > newRowDiff) ||(smallestColDiff > newColDiff)){
	smallestRowDiff = abs(monster->lastKnownPosOfPC.row - neighbors[i].row);
	smallestColDiff = abs(monster->lastKnownPosOfPC.col - neighbors[i].col);
	targetCoord.row = neighbors[i].row;
	targetCoord.col = neighbors[i].col;
      }
    }
    targetRow = targetCoord.row;
    targetCol = targetCoord.col;
  }
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

int proximityCheck(Dungeon *d, Character *monster){
  int rowDifference = abs(d->pc.location.row - monster->location.row);
  int colDifference = abs(d->pc.location.col - monster->location.col);
  if((rowDifference < 6) && (colDifference < 6)){
    return 0;
  }
  return 1;
}

void moveCharacter(Dungeon *d, int targetRow, int targetCol, Character *curr){
  if(curr->alive == true){
    if(curr->location.row != targetRow || curr->location.col != targetCol){
      if(curr->type == '@'){
	if(d->characterMap[targetRow][targetCol] != '\0'){
	  uint8_t i;
	  for(i = 0; i < d->numOfMonsters; i++){
	    if(d->monsters[i].location.row == targetRow && d->monsters[i].location.col == targetCol){
	      d->characterMap[targetRow][targetCol] = '\0';
	      d->monsters[i].alive = 1;
	    }
	  }
	}
      }
      else if(curr->type == '*'){}
      else {
	if(d->hardnessMap[curr->location.row][curr->location.col] != MIN_HARDNESS){
	  d->map[curr->location.row][curr->location.col] = '#';
	  d->hardnessMap[curr->location.row][curr->location.col] = 0;
	}
	if(d->characterMap[targetRow][targetCol] != '\0'){
	  //check for player
	  if(d->pc.location.row == targetRow && d->pc.location.col == targetCol){
	    d->pc.alive = false;
	  }else {
	    uint8_t i;
	    for(i = 0; i < d->numOfMonsters; i++){
	      if(d->monsters[i].location.row == targetRow && d->monsters[i].location.col == targetCol){
		d->characterMap[targetRow][targetCol] = '\0';
		d->monsters[i].alive = false;
		break;
	      }
	    }
	  }
	}
      }
    }
    sortDeadMonsters(d);
    d->characterMap[curr->location.row][curr->location.col] = '\0';
    d->characterMap[targetRow][targetCol] = curr->type;
    curr->location.row = targetRow;
    curr->location.col = targetCol;
  }
  
}

void sortDeadMonsters(Dungeon *d){
  uint8_t i;
  for(i = 0; i < d->numOfMonsters - 1; i++){
    if(d->monsters[i].alive == false && d->monsters[i + 1].alive == true){
      Character tempMonster = d->monsters[i];
      d->monsters[i] = d->monsters[i + 1];
      d->monsters[i + 1] = tempMonster;
    }
  }
}

bool checkTargetCoordinateTeleport(Dungeon *d, uint8_t row, uint8_t col){
  if(row > 0 && col > 0 && row < 20 && col < 79){
    return true;
  }
  return false;
}

bool checkTargetCoordinate(Dungeon *d, uint8_t row, uint8_t col){
  if(checkTargetCoordinateTeleport(d, row, col) == true && d->hardnessMap[row][col] == MIN_HARDNESS){
    return true;
  }
  return false;
}

