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

void placeCharacter(Dungeon *d, int row, int col){
  d->characterMap[row][col] = '@';
  d->pc.location.row = row;
  d->pc.location.col = col;
}

int main(int argc, char *argv[]){
  int seed = time(NULL);
  if(argc == 2 && argv[1][0] != '-') seed  = atoi(argv[1]);
  printf("Seed used: %d\n", seed);
  srand(seed);
  Dungeon d;
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
      placeCharacter(&d, row, col);
  }

  generateNormalPathMap(&d);
  generateTunnelPathMap(&d);
  
  printf("Map Created\n");
  printMap(&d);
  
  if(numOfMonsters == 0){
    numOfMonsters = d.numOfRooms;
  }
  d.numOfMonsters = numOfMonsters;
  d.monsters = (Character*)malloc(numOfMonsters * sizeof(Character));
  makeMonsters(&d);
  placeMonsters(&d);
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
    }else{
      //a monster, so move it
      moveMonster(d, curr);
    }


  }
  heap_delete(&h);
}

void moveCharacter(Dungeon *d, int row, int col, Character *curr){
  if(curr->alive == TRUE){
    if(d->hardnessMap[curr->location.row][curr->location.col] != MIN_HARDNESS){
      d->map[curr->location.row][curr->location.col] = '#';
      d->hardnessMap[curr->location.row][curr->location.col] = 0;
    }
    if(d->characterMap[row][col] != '\0'){
      if(curr->representation != '@'){
	if(d->pc.location.row == row && d->pc.location.col == col){
	  d->pc.alive = FALSE;
	}else{
	  int i;
	  for(i = 0; i < d->numOfMonsters; i++){
	    if(d->monsters[i].location.row == row && d->monsters[i].location.col == col){
	      d->monsters[i].alive = FALSE;
	    }
	  }
	}
	d->characterMap[row][col] = '\0';
      }
    }
    d->characterMap[curr->location.row][curr->location.col] = '\0';
    d->characterMap[row][col] = curr->representation;
    curr->location.row = row;
    curr->location.col = col;
    if(curr->representation == '@'){
      printMap(d);
    }
  }
}

void createDungeon(Dungeon *d){
  initlizeDungeon(d);
  d->rooms = (Room*)malloc(MAX_NUMBER_OF_ROOMS * sizeof(Room));
  generateRooms(d);
  placeRooms(d);
  setSortedRoomArray(d);
  connectRooms(d);
}

void makeMonsters(Dungeon *d){
  int i;
  for(i = 0; i < d->numOfMonsters; i++){
    if(i % 2 == 0){
      d->monsters[i].characteristic = 0x3;
    }else{
      d->monsters[i].characteristic = 0x7;
    }
    //d->monsters[i].characteristic = rand() & 0xf;
    d->monsters[i].speed = generateRange(5, 21);
    d->monsters[i].alive = TRUE;
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
    if(!placed){
      int randomRoom = generateRange(1, d->numOfRooms);
      Room currRoom = d->rooms[randomRoom];
      int randomRow = generateRange(0, currRoom.height);
      int randomCol = generateRange(0, currRoom.width);
      int selectedRow = currRoom.topLeftCoord.row + randomRow;
      int selectedCol = currRoom.topLeftCoord.col + randomCol;
      if(d->characterMap[selectedRow][selectedCol] == '\0'){
	d->characterMap[selectedRow][selectedCol] = d->monsters[i].representation;
	d->monsters[i].location.row = selectedRow;
	d->monsters[i].location.col = selectedCol;
	placed = 1;
      }
    }
  }
}

void moveMonster(Dungeon *d, Character *monster){
  /*
 if(isInSameRoom(d, monster) == TRUE){
    monster->lastKnownPosOfPC = d->pc.location;
  }
  */
  int currRow, currCol, targetRow, targetCol, i, j;
  currRow = monster->location.row;
  currCol = monster->location.col;
  int addRow, addCol, placed;
  int chance = rand() % 2;
  switch(monster->representation){
    //just line of sight
  case '0':
    if(monster->lastKnownPosOfPC.row == 0 && monster->lastKnownPosOfPC.col == 0){
      int viableSpot = 0;
      if(viableSpot){
	addRow = rand() % 2;
	addCol = rand() % 2;
	currRow = monster->location.row;
	currCol = monster->location.col;
	if(d->hardnessMap[currRow + addRow][currCol + addCol] == MIN_HARDNESS){
	  moveCharacter(d, currRow + addRow, currCol + addCol, monster);
	  viableSpot = 1;
	}
	else if(d->hardnessMap[currRow + addRow][currCol - addCol] == MIN_HARDNESS){
	  moveCharacter(d, currRow + addRow, currCol - addCol, monster);
	  viableSpot = 1;
	}
	else if(d->hardnessMap[currRow - addRow][currCol + addCol] == MIN_HARDNESS){
	  moveCharacter(d, currRow - addRow, currCol + addCol, monster);
	  viableSpot = 1;
	}
	else if(d->hardnessMap[currRow - addRow][currCol - addCol] == MIN_HARDNESS){
	  moveCharacter(d, currRow - addRow, currCol - addCol, monster);
	  viableSpot = 1;
	}
      }
    }
    break;
  case '1': //0001
    //intel but only line of sight
    /*
    if(d->monsters[selected].lastKnownPosOfPC.row != 0){
      if(d->monsters[selected].lastKnownPosOfPC.row < d->monsters[selected].location.row){
	
      }
    }
    */
    break;
  case '2': //0010
    //a telepath
    //always knows where player is, but not short path
    if(monster->lastKnownPosOfPC.row > currRow && d->hardnessMap[currRow + 1][currCol] == MIN_HARDNESS){
      targetRow = currRow + 1;
    }
    else if(monster->lastKnownPosOfPC.row < currRow && d->hardnessMap[currRow - 1][currCol] == MIN_HARDNESS){
      targetRow = currRow - 1;
    }
    else if(monster->lastKnownPosOfPC.col < currCol && d->hardnessMap[currRow][currCol - 1] == MIN_HARDNESS){
      targetCol = currCol - 1;
    }
    else if(monster->lastKnownPosOfPC.col > currCol && d->hardnessMap[currRow][currCol + 1] == MIN_HARDNESS){
      targetCol = currCol + 1;
    }
    moveCharacter(d, targetRow, targetCol, monster);
    break;
  case '3': //0011
    //tele and smart (imp)
    currRow = monster->location.row;
    currCol = monster->location.col;
    int smallestValue = d->nonTunnelPaths[currRow][currCol];
    targetRow = currRow;
    targetCol = currCol;
    for(i = -1; i <= 1; i++){
      for(j = -1; j<= 1; j++){
	if(d->hardnessMap[currRow + i][currCol + j] == MIN_HARDNESS){
	  int tmpValue = d->nonTunnelPaths[currRow + i][currCol + j];
	  if(tmpValue < smallestValue){
	    smallestValue = tmpValue;
	    targetRow = currRow + i;
	    targetCol = currCol + j;
	  }
	}
      }
    }
    moveCharacter(d, targetRow, targetCol, monster);
    break;
  case '4': //0100
    //telepath
    break;
  case '5': //0101
    //tunnel and smart
    break;
  case '6': //0110
    //tele and tunnel
    break;
  case '7': //0111
    //tunnel, tele, and intelligent (imp)
    currRow = monster->location.row;
    currCol = monster->location.col;
    int smallest = d->tunnelPaths[currRow][currCol];
    for(i = -1; i <= 1; i++){
      for(j = -1; j<= 1; j++){
	if(d->hardnessMap[currRow + i][currCol + j] != MAX_HARDNESS){
	  int tmpValue = d->tunnelPaths[currRow + i][currCol + j];
	  if(tmpValue < smallest){
	    smallest = tmpValue;
	    targetRow = currRow + i;
	    targetCol = currCol + j;
	  }
	}
      }
    }
    moveCharacter(d, targetRow, targetCol, monster);
    break;
  case '8': //1000
    if(chance == 1 && monster->lastKnownPosOfPC.row != 0){
      if(monster->lastKnownPosOfPC.row > currRow && d->hardnessMap[currRow + 1][currCol] == MIN_HARDNESS){
	targetRow = currRow + 1;
      }
      else if(monster->lastKnownPosOfPC.row < currRow && d->hardnessMap[currRow - 1][currCol] == MIN_HARDNESS){
	targetRow = currRow - 1;
      }
      else if(monster->lastKnownPosOfPC.col < currCol && d->hardnessMap[currRow][currCol - 1] == MIN_HARDNESS){
	targetCol = currCol - 1;
      }
      else if(monster->lastKnownPosOfPC.col > currCol && d->hardnessMap[currRow][currCol + 1] == MIN_HARDNESS){
	targetCol = currCol + 1;
      }
    }else{
      placed = 0;
      if(placed){
	addRow = generateRange(-1, 2);
	addCol = generateRange(-1, 2);
	if(d->hardnessMap[currRow + addRow][currCol + addCol] == MIN_HARDNESS){
	  placed = 1;
	  targetRow = currRow + addRow;
	  targetCol = currRow + addCol;
	}
      }
    }
  moveCharacter(d, targetRow, targetCol, monster);
    break;
  case '9': //1001
    //erratic but smart
    break;
  case 'a': //1010
    //tunnel and telepath
    break;
  case 'b': //1011
    //erratic, telepath and smart
    break;
  case 'c': //1100
    //erratic tunnler
    break;
  case 'd': //1101
    //erratic, tunnel, smart
    break;
  case 'e': //1110
    //erratic, tunnel, telepath
    break;
  case 'f': //1111
    //erratic, tunnel, telepath, smart
    break;
  }
  
}


int isInSameRoom(Dungeon *d, Character *monster){
  int i;
  for(i = 0; i < d->numOfRooms; i++){
    int playerHere = FALSE;
    int monsterHere = FALSE;
    int currRow;
    int currCol;
    for(currRow = d->rooms[i].topLeftCoord.row; currRow < d->rooms[i].topLeftCoord.row +
	  d->rooms[i].height; currRow++){
      for(currCol = d->rooms[i].topLeftCoord.col; currCol < d->rooms[i].topLeftCoord.row +
	    d->rooms[i].width; currCol++){
	if(currRow == d->pc.location.row && currCol == d->pc.location.col){
	  playerHere = TRUE;
	}
	if(currRow == monster->location.row && 
	   currCol == monster->location.col){
	  monsterHere = TRUE;
	}
      }
    }
    if(playerHere == TRUE && monsterHere == TRUE){
      return TRUE;
    }else {
      playerHere = monsterHere = FALSE;
    }
  }

  return FALSE;
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
  placeCharacter(d, rowOfPlayer, colOfPlayer);
  fclose(fp);
}
