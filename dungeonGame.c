#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <endian.h>
#include <inttypes.h>
#include <unistd.h>

#include "dungeonInfo.h"
#include "buildDungeon.h"
#include "pathFinding.h"
#include "character.h"

const char roomCell = '.';
const char corridorCell = '#';
const char playerCell = '@';
const char rockCell = ' ';

void printMap(Dungeon *d);
void printPaths(Dungeon *d);
void printTunnelPaths(Dungeon *d);
void srand(unsigned seed);
void createDungeon(Dungeon *d);
void saveGame(Dungeon *d);
void loadGame(Dungeon *d);
void runGame(Dungeon *d);
void makeMonsters(Dungeon *d);
void placeMonsters(Dungeon *d);
int isInSameRoom(Dungeon *d, Character *monster);
void moveMonster(Dungeon *d, Character *monster);
void moveCharacter(Dungeon *d, int row, int col, Character *curr);

void placeCharacter(Dungeon *d, int row, int col, Character *curr){
  d->characterMap[row][col] = curr->representation;
  curr->location.row = row;
  curr->location.col = col;
}

int main(int argc, char *argv[]){
  int seed = time(NULL);
  if(argc == 2 && argv[1][0] != '-') seed  = atoi(argv[1]);
  srand(seed);
  Dungeon d;
  printf("Seed used: %d\n", seed);
  int isLoad = FALSE;
  int isSave = FALSE;
  int numOfMonsters = 0;

  if(argc > 2){
    if((argv[1][2] == 'l' && argv[2][2] == 's') || (argv[1][2] == 's' && argv[2][2] == 'l')){
      printf("User selected to both save and load\n");
      isLoad = TRUE;
    }
    //check for num of monsters
    else if(argv[1][2] == 'n'){
      numOfMonsters = atoi(argv[2]);
    }
  } 
    
  if(argc == 2 && argv[1][0] == '-'){
    if(argv[1][2] == 's'){
      printf("User selected save\n");
      isSave = TRUE;
    }
    else if(argv[1][2] == 'l'){
      printf("User selected load\n");
      isLoad = TRUE;
    }
  }

  if(isLoad == TRUE){
    initlizeDungeon(&d);
    loadGame(&d);
  } else{
      createDungeon(&d);
      int row = d.rooms[0].topLeftCoord.row;
      int col = d.rooms[0].topLeftCoord.col;
      d.pc.alive = TRUE;
      d.pc.representation = playerCell;
      d.pc.location.row = row;
      d.pc.location.col = col;
      d.pc.speed = generateRange(5, 21);
      d.pc.turn = 1000/ d.pc.speed;
      d.characterMap[row][col] = '@';
      placeCharacter(&d, row, col, &d.pc);
  }

  generateNormalPathMap(&d);
  generateTunnelPathMap(&d);
  
  printf("Map Created\n");
  //printMap(&d);
  
  if(numOfMonsters == 0){
    numOfMonsters = d.numOfRooms;
  }
  d.numOfMonsters = numOfMonsters;
  d.monsters = (Character*)malloc(numOfMonsters * sizeof(Character));
  makeMonsters(&d);
  placeMonsters(&d);
  printMap(&d);
  while(d.pc.alive == TRUE){
    runGame(&d);
    usleep(250000);
  }
  printMap(&d);
  printf("GAME OVER\n");
  free(d.rooms);
  free(d.monsters);
  
  if(isSave == TRUE){
    saveGame(&d);
  }
  printf("Seed used: %d\n", seed);
  return 0;
}

static int32_t moveComparator(const void *key, const void *with){
  return((Character *) key)->turn - ((Character *) with)->turn;
}

void runGame(Dungeon *d){
  heap_t h;
  heap_init(&h, moveComparator, NULL);
  //fill heap
  d->pc.hn = heap_insert(&h, &d->pc);
  int i;
  for(i = 0; i < d->numOfMonsters; i++){
    d->monsters[i].hn = heap_insert(&h, &d->monsters[i]);
  }

  Character *curr;
  while((curr = heap_remove_min(&h))){
    curr->hn = NULL;
    if(curr->representation == '@'){   
      moveCharacter(d, d->pc.location.row, d->pc.location.col, &d->pc);
      printMap(d);
      //update distance maps
    }else{
      //a monster, so move it
      moveMonster(d, curr);
    }


  }
  heap_delete(&h);
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
      if(d->characterMap[row][col] != '\0'){
	printf("%c", d->characterMap[row][col]);
      }else {
	printf("%c", d->map[row][col]);
      }
    }
    printf("\n");
  }
}

void printPaths(Dungeon *d){
  uint8_t x, y;
  for( y = 0; y < DUNGEON_HEIGHT; y++){
    for(x = 0; x < DUNGEON_WIDTH; x++){
      if(y == d->pc.location.row && x == d->pc.location.col){
	printf("@");
      }else {
	if(d->hardnessMap[y][x] != MIN_HARDNESS){
	  printf(" ");
	}else{
	  printf("%d", d->nonTunnelPaths[y][x] % 10);	
	}
      }
    }
    printf("\n");
  }
}

void printTunnelPaths(Dungeon *d){
  uint8_t x, y;
  for( y = 0; y < DUNGEON_HEIGHT; y++){
    for(x = 0; x < DUNGEON_WIDTH; x++){
      if(y == d->pc.location.row && x == d->pc.location.col){
	printf("@");
      }else {
	if(d->hardnessMap[y][x] == MAX_HARDNESS){
	  printf(" ");
	}else{
	  printf("%d", d->tunnelPaths[y][x] % 10);	
	}
      }
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
      fread(&d->hardnessMap[i][j], sizeof(uint8_t), 1, fp);
      if(d->hardnessMap[i][j] == MIN_HARDNESS){
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

  d->pc.representation = '@';
  Character curr = d->pc;
  placeCharacter(d, rowOfPlayer, colOfPlayer, &curr);
  fclose(fp);
}
