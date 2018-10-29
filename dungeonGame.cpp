#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <endian.h>
#include <inttypes.h>
#include <unistd.h>
#include <ncurses.h>

#include "dungeonInfo.h"
#include "buildDungeon.h"
#include "pathFinding.h"
#include "character.h"
#include "io.h"

const char roomCell = '.';
const char corridorCell = '#';
const char playerCell = '@';
const char rockCell = ' ';

void printMap(Dungeon *d);
void srand(unsigned seed);
void saveGame(Dungeon *d);
void loadGame(Dungeon *d);
void runGame(Dungeon *d);

void placeCharacter(Dungeon *d, int row, int col, Character *curr){
  d->characterMap[row][col] = curr->type;
  curr->location.row = row;
  curr->location.col = col;
}

int main(int argc, char *argv[]){
  int seed = time(NULL);
  if(argc == 2 && argv[1][0] != '-') seed  = atoi(argv[1]);
  srand(seed);
  Dungeon d;
  printf("Seed used: %d\n", seed);
  int isLoad = FAILURE;
  int isSave = FAILURE;
  int numOfMonsters = 0;

  if(argc > 2){
    if((argv[1][2] == 'l' && argv[2][2] == 's') || (argv[1][2] == 's' && argv[2][2] == 'l')){
      printf("User selected to both save and load\n");
      isLoad = SUCCESS;
    }
    //check for num of monsters
    else if(argv[1][2] == 'n'){
      numOfMonsters = atoi(argv[2]);
    }
  } 
    
  if(argc == 2 && argv[1][0] == '-'){
    if(argv[1][2] == 's'){
      printf("User selected save\n");
      isSave = SUCCESS;
    }
    else if(argv[1][2] == 'l'){
      printf("User selected load\n");
      isLoad = SUCCESS;
    }
  }

  if(isLoad == SUCCESS){
    initlizeDungeon(&d);
    loadGame(&d);
  } else{
    createDungeon(&d);
    int row = d.rooms[0].topLeftCoord.row;
    int col = d.rooms[0].topLeftCoord.col;
    d.pc.alive = true;
    d.pc.type = playerCell;
    d.pc.location.row = row;
    d.pc.location.col = col;
    d.pc.speed = generateRange(5, 21);
    d.pc.turn = 1000/ d.pc.speed;
    d.characterMap[row][col] = '@';
    placeCharacter(&d, row, col, &d.pc);
  }

  generateNormalPathMap(&d);
  generateTunnelPathMap(&d);
  
  if(numOfMonsters == 0){
    numOfMonsters = d.numOfRooms;
  }
  
  d.fogOfWar = true;
  d.numOfMonsters = numOfMonsters;
  d.monsters = (Character*)malloc(numOfMonsters * sizeof(Character));
  makeMonsters(&d);
  placeMonsters(&d);
  
  initTerminal();
  
  newPrintMap(&d);
  while(d.pc.alive == true && d.monsters[0].alive == true){
      d.runs++;
      runGame(&d); 
  }
  endwin();
  printf("GAME OVER\n");
  if(d.monsters[0].alive == false){
    printf("You Win!\n");
  }else {
    printf("If you want to win next time just remember you don't have to be perfect... \n");
    printf("But you need to be a lot better than that.\n");
  }
  free(d.rooms);
  free(d.monsters);
  

  if(isSave == SUCCESS){
    saveGame(&d);
  }
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
  uint8_t i;
  for(i = 0; i < d->numOfMonsters; i++){
      d->monsters[i].hn = heap_insert(&h, &d->monsters[i]);
  }
  Character *curr;
  while((curr = (Character *)heap_remove_min(&h))){
    curr->hn = NULL;
    if(curr->type == '@'){   
      handleUserInput(d);
      newPrintMap(d);
    }else{
	moveMonster(d, curr);
    }
  }
  heap_delete(&h);
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

  uint8_t i,j;
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

  uint8_t i, j;
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

  d->pc.type = '@';
  Character curr = d->pc;
  placeCharacter(d, rowOfPlayer, colOfPlayer, &curr);
  fclose(fp);
}
