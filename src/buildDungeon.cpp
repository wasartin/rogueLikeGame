#include <stdio.h>
#include <stdlib.h>

#include "dungeonInfo.h"
#include "buildDungeon.h"
#include "utils.h"

void initlizeDungeon(Dungeon *d){
  printf("Initilizing Dungeon...\n");
  int row, col;
  char topBorder = '-';
  char sideBorder = '|';
  char rockCell = ' ';
  for(row = 0; row < DUNGEON_HEIGHT; row++){
    for(col = 0; col < DUNGEON_WIDTH; col++){
      d->characterMap[row][col] = '\0';
      if(row == 0 || row == 20){
	d->map[row][col] = topBorder;
	d->hardnessMap[row][col] = MAX_HARDNESS;
      }
      else if(col == 0 || col == 79){
	d->map[row][col] = sideBorder;
	d->hardnessMap[row][col] = MAX_HARDNESS;
      }
      else{
	d->map[row][col] = rockCell;
	d->hardnessMap[row][col] = generateRange(MIN_HARDNESS + 1, MAX_HARDNESS);
      }
    }
  }
}

void createDungeon(Dungeon *d){
  initlizeDungeon(d);
  d->runs = 0;
  d->rooms = (Room*)malloc(MAX_NUMBER_OF_ROOMS * sizeof(Room));
  generateRooms(d);
  placeRooms(d);
  setSortedRoomArray(d);
  connectRooms(d);
  generateStairs(d, d->rooms[0].topLeftCoord.row, d->rooms[0].topLeftCoord.col, 0);
  generateStairs(d, d->rooms[d->numOfRooms - 1].topLeftCoord.row, d->rooms[d->numOfRooms - 1].topLeftCoord.col, 1);
}

void generateRooms(Dungeon *d){
  int qtyOfRooms = generateRange(MIN_NUMBER_OF_ROOMS, MAX_NUMBER_OF_ROOMS);
  d->numOfRooms = qtyOfRooms;
  
  int i;
  for(i = 0; i < qtyOfRooms; i++){
    int rows = generateRange(MIN_ROOM_HEIGHT, MAX_ROOM_HEIGHT);
    int columns = generateRange(MIN_ROOM_WIDTH, MAX_ROOM_WIDTH);
    Room currRoom;
    currRoom.width = columns;
    currRoom.height = rows;
    currRoom.isConnected = FAILURE;
    d->rooms[i] = currRoom;
  }
}

int isInBounds(int row, int col, Room *r){
  //check if in the immutable section
  if(row == 0 || row == 79 || col == 0 || col == 20) return FAILURE;
  //check bounds
  if(row + r->height > 19) return FAILURE;
  if(col + r->width > 78) return FAILURE;
  return SUCCESS;
}

int checkAdjaceny(int row, int col,  Dungeon *d, char cellToLookFor){
  //visited order
  //    0 7 6
  //    1 - 5
  //    2 3 4
  int adjacentCells = 8;
  int xRoutes[] = {-1,  0,  1, 1, 1, 0, -1, -1};
  int yRoutes[] = {-1, -1, -1, 0, 1, 1,  1,  0};
  int i, j;
  for(i = 0; i < adjacentCells; i++){
    for(j = 0; j < adjacentCells; j++){
      if(d->map[row + xRoutes[j]][col + yRoutes[j]] == cellToLookFor) return FAILURE;
    }
  }
  return SUCCESS;
}

/*
 *Kind of messy. setting the points when checking if this is a legal place.
 */
int isLegalPlace(int row, int col, Dungeon *d, Room *r){
  if(isInBounds(row, col, r) == FAILURE) return FAILURE;
  Coordinate tempCoord;
  tempCoord.row = row;
  tempCoord.col = col;
  r->topLeftCoord = tempCoord;
  int i, j;
  char roomCell = '.';
  for(i = row; i < row + r->height; i++){
    for(j = col; j < col + r->width; j++){
      if(checkAdjaceny(i, j, d, roomCell) == FAILURE) return FAILURE;
    }
  }
  return SUCCESS;
}

void placeRooms(Dungeon *d){
  printf("Placing Rooms inside Dungeon...\n");
  uint8_t roomNumber = 0;
  int attempts = 0;
  int attemptLimit = 2000;
  while(roomNumber < d->numOfRooms){
    int randomRow = (rand() % 19) + 1;
    int randomCol = (rand() % 78) + 1;
    if(isLegalPlace(randomRow, randomCol, d, &(d->rooms[roomNumber])) == SUCCESS){
      
      fillMap(d, roomNumber);
      roomNumber++;
    }
    attempts++;
    //should just start over if this happens, like print out "Error occured. Reconfiguring..."
    if(attempts > attemptLimit){
      printf("FAILURE TO PLACE ALL ROOMS\n");
      break;
    }
  }
}
 
void fillMap(Dungeon *d, int roomNumber){
  int i, j;
  char roomCell = '.';
  uint8_t row = d->rooms[roomNumber].topLeftCoord.row;
  uint8_t col = d->rooms[roomNumber].topLeftCoord.col;
  for(i = row; i < row + d->rooms[roomNumber].height; i++){
    for(j = col; j < col + d->rooms[roomNumber].width; j++){
      d->map[i][j] = roomCell;
      d->hardnessMap[i][j] = MIN_HARDNESS;
    }
  }
}
void setSortedRoomArray(Dungeon *d){
  uint8_t i;
  //sort from top left to bottom right
  for(i = 0; i < d->numOfRooms; i++){
    uint8_t j;
    int minIndex = i;
    for(j = i + 1; j <  d->numOfRooms; j++){
      if(d->rooms[minIndex].topLeftCoord.col == d->rooms[j].topLeftCoord.col){
	if(d->rooms[j].topLeftCoord.row < d->rooms[minIndex].topLeftCoord.row){
	  minIndex = j;
	}
      }
      //compare col
      else{
	if(d->rooms[j].topLeftCoord.col < d->rooms[minIndex].topLeftCoord.col){
	minIndex = j;
	}
      }
    }
    Room temp = d->rooms[minIndex];
    d->rooms[minIndex] = d->rooms[i];
    d->rooms[i] = temp;
  }
}

void connectRooms(Dungeon *d){
  uint8_t i;
  int attempts = 0;
  int attemptLimit = 2000;
  char corridorCell = '#';
  for(i = 0; i < d->numOfRooms - 1; i++){
    Room currRoom = d->rooms[i];
    if(currRoom.isConnected == FAILURE){
      Room nextRoom = d->rooms[i + 1];

      //new
      int randomRow = generateRange(0, currRoom.height);
      int randomCol = generateRange(0, currRoom.width);
      Coordinate currCoord;
      currCoord.row = currRoom.topLeftCoord.row + randomRow;
      currCoord.col = currRoom.topLeftCoord.col + randomCol;

      int nextRow = generateRange(0, nextRoom.height);
      int nextCol = generateRange(0, nextRoom.width);
      Coordinate destinationCoord;
      destinationCoord.row = nextRoom.topLeftCoord.row + nextRow;
      destinationCoord.col = nextRoom.topLeftCoord.col + nextCol;

      while((currCoord.row != destinationCoord.row) || (currCoord.col != destinationCoord.col)){
	attempts++;
	if(attempts > attemptLimit){
	  printf("FAILURE TO CONNECT ALL ROOMS\n");
	  break;
	}
	int rowDifference, colDifference;
	rowDifference = abs(destinationCoord.row - currCoord.row);
	colDifference = abs(destinationCoord.col - currCoord.col);
	if(rowDifference > colDifference){

	  if(currCoord.row < destinationCoord.row){
	    currCoord.row += 1;
	  }
	  else{
	    currCoord.row -= 1;	  }
	}
	else {
	  if(currCoord.col < destinationCoord.col){
	    currCoord.col += 1;
	  }
	  else {
	    currCoord.col -= 1;
	  }
	}
	if(d->map[currCoord.row][currCoord.col] == ' '){
	  d->map[currCoord.row][currCoord.col] = corridorCell;
	  d->hardnessMap[currCoord.row][currCoord.col] = MIN_HARDNESS;
	}
      }
      d->rooms[i].isConnected = SUCCESS;
    }
  }
}


void generateStairs(Dungeon *d, int row, int col, int dir){
  //if 0, go upstairs
  if(dir == 0){
    d->map[row][col] = '<';
  }else{
    d->map[row][col] = '>';
  }
}
