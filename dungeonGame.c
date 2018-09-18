#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <endian.h>

#include "dungeon.h"
#include "buildDungeon.h"

const char roomCell = '.';
const char corridorCell = '#';
const char playerCell = '@';
const char rockCell = ' ';

void printMap(Dungeon *d);
void srand(unsigned seed);
void createDungeon(Dungeon *d);
void saveGame(Dungeon *d);
void loadGame(Dungeon *d);
void placeCharacter(Dungeon *d, int row, int col);

int main(int argc, char *argv[]){
  int seed = time(NULL);
  if(argc == 2 && argv[1][0] != '-') seed  = atoi(argv[1]);
  printf("Seed used: %d\n", seed);
  srand(seed);
  Dungeon d;
  int isLoad = FALSE;
  int isSave = FALSE;

  if(argc > 2){
    if((argv[1][2] == 'l' && argv[2][2] == 's') || (argv[1][2] == 's' && argv[2][2] == 'l')){
      printf("User selected to both save and load\n");
      isLoad = TRUE;
    }
  } 
    
  if(argc == 2 && argv[1][0] == '-'){
    if(argv[1][2] == 's'){
      printf("User selected save\n");
      isSave = TRUE;
    }
    else if(argv[1][2] == 'l'){
      isLoad = TRUE;
    }
  }

  if(isLoad == TRUE){
    loadGame(&d);
  } else{
      createDungeon(&d);
      int row = d.rooms[0].topLeftCoord.row;
      int col = d.rooms[0].topLeftCoord.col;
      placeCharacter(&d, row, col);
  }
  if(isSave == TRUE){
    saveGame(&d);
  }
 
  free(d.rooms);

  printf("Map Created\n");
  printMap(&d);
  return 0;
}

void createDungeon(Dungeon *d){
  initlizeDungeon(d);
  d->rooms = (Room*)malloc(MAX_NUMBER_OF_ROOMS * sizeof(Room));
  generateRooms(d);
  placeRooms(d);
  setSortedRoomArray(d);
  connectRooms(d);
}

void printMap(Dungeon *d){
  int row, col;
  for(row = 0; row < DUNGEON_HEIGHT; row++){
    for(col = 0; col < DUNGEON_WIDTH; col++){
      printf("%c", d->map[row][col]);
    }
    printf("\n");
  }
}

void saveGame(Dungeon *d){
  FILE *fp;
  char *filePath = getenv("HOME");
  strcat(filePath, "/.rlg327/dungeon");
  fp = fopen(filePath, "wb");
  char marker[] = "RLG327-F2018";
  fwrite(&marker, sizeof(marker) - 1, 1, fp);

  uint32_t versionNumber = htobe32(0);
  fwrite(&versionNumber, sizeof(int32_t), 1, fp);

  uint32_t fileSize = 1702 + (d->numOfRooms * 4);
  fileSize = htobe32(fileSize);
  fwrite(&fileSize, sizeof(uint32_t), 1, fp);
  
  uint8_t row = d->pc.location.row;
  uint8_t col = d->pc.location.col;
  fwrite(&col, sizeof(uint8_t), 1, fp);
  fwrite(&row, sizeof(uint8_t), 1, fp);

  int i,j;
  for(i = 0; i < DUNGEON_HEIGHT; i++){
    for(j = 0; j < DUNGEON_WIDTH; j++){
      fwrite(&d->hardnessMap[i][j], sizeof(uint8_t), 1, fp);
    }
  }

  for(i = 0; i < d->numOfRooms; i++){
    uint8_t currInfo;
    currInfo = d->rooms[i].topLeftCoord.col;
    fwrite(&currInfo, sizeof(uint8_t), 1, fp);
    currInfo = d->rooms[i].topLeftCoord.row;
    fwrite(&currInfo, sizeof(uint8_t), 1, fp);
    currInfo = d->rooms[i].width;
    fwrite(&currInfo, sizeof(uint8_t), 1, fp);
    currInfo = d->rooms[i].height;
    fwrite(&currInfo, sizeof(uint8_t), 1, fp);
  }
  fclose(fp);
}

void loadGame(Dungeon *d){
  FILE *fp;
  char *filePath = getenv("HOME");
  strcat(filePath, "/.rlg327/dungeon");
  fp = fopen(filePath, "rb");

  char marker[13];
  marker[12] = '\0';
  fread(marker, sizeof(marker) - 1, 1 , fp);

  uint32_t versionNumber;
  fread(&versionNumber, sizeof(uint32_t), 1, fp);
  versionNumber = be32toh(versionNumber);

  uint32_t lengthOfFile;
  fread(&lengthOfFile, sizeof(uint32_t), 1, fp);
  lengthOfFile = be32toh(lengthOfFile);

  uint8_t rowOfPlayer, colOfPlayer;
  fread(&colOfPlayer, sizeof(uint8_t), 1, fp);
  fread(&rowOfPlayer, sizeof(uint8_t), 1, fp);

  int i, j;
  for(i = 0; i < DUNGEON_HEIGHT; i++){
    for(j = 0; j < DUNGEON_WIDTH; j++){
      //write in hardness map, while you are reading it
      fread(&d->hardnessMap[i][j], sizeof(uint8_t), 1, fp);
      if(d->hardnessMap[i][j] == MIN_HARDNESS){
	//if 0, then it is either a room or a corridor so just write corridor for now
	d->map[i][j] = corridorCell;
      }
      else{
	d->map[i][j] = rockCell;
      }
    }
  }

  uint8_t xpos, ypos, width, height;
  uint32_t setLength = 1702;
  uint32_t numOfRoomsInFile = (lengthOfFile - setLength) / 4;
  
  d->rooms = (Room*)malloc(numOfRoomsInFile * sizeof(Room));
  for(i = 0; i < numOfRoomsInFile; i++){
    fread(&xpos, sizeof(uint8_t), 1, fp);
    fread(&ypos, sizeof(uint8_t), 1, fp);
    fread(&width, sizeof(uint8_t), 1, fp);
    fread(&height, sizeof(uint8_t), 1, fp);
    Room currRoom;
    Coordinate coord;
    coord.row = ypos;
    coord.col = xpos;
    currRoom.topLeftCoord = coord;
    currRoom.width = width;
    currRoom.height = height; 
    d->rooms[i] = currRoom;
    fillMap(d, i);
  }
 
  placeCharacter(d, rowOfPlayer, colOfPlayer);
  fclose(fp);
}

void placeCharacter(Dungeon *d, int row, int col){
  d->pc.location.row = row;
  d->pc.location.col = col;
  d->map[row][col] = playerCell;
}
