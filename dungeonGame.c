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

const char roomCell = '.';
const char corridorCell = '#';
const char playerCell = '@';
const char rockCell = ' ';

void printMap(Dungeon *d);
void newPrintMap(Dungeon *d);
void srand(unsigned seed);
void createDungeon(Dungeon *d);
void saveGame(Dungeon *d);
void loadGame(Dungeon *d);
void runGame(Dungeon *d, char userInput);
void action(char action, int *addRow, int *addCol);

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
  STAIRS:
    createDungeon(&d);
    int row = d.rooms[0].topLeftCoord.row;
    int col = d.rooms[0].topLeftCoord.col;
    d.pc.alive = 0;
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
  
  if(numOfMonsters == 0){
    numOfMonsters = d.numOfRooms;
  }
  d.numOfMonsters = numOfMonsters;
  d.monsters = (Character*)malloc(numOfMonsters * sizeof(Character));
  makeMonsters(&d);
  placeMonsters(&d);
  
  //clears screen and allocates memory for screen
  WINDOW *base = initscr();
  raw();
  noecho();
  cbreak();
  curs_set(0);
  keypad(base, TRUE);

  newPrintMap(&d);
  while(d.pc.alive == 0 && d.monsters[0].alive == 0){
    char ch = getch();
    if((ch == '>' && d.map[d.pc.location.row][d.pc.location.col] == '>') ||
       (ch == '<' && d.map[d.pc.location.row][d.pc.location.col] == '<')){
      free(d.rooms);
      free(d.monsters);
      goto STAIRS;
    }
    else if(ch == 'm'){
      int i;
      int screenLimit = 10;
      if(d.numOfMonsters < 10){
	screenLimit = d.numOfMonsters;
      }
      int shift = 0;
      int inMonsterScreen = 0;
      clear();
      while(inMonsterScreen == 0){
	for(i = 0; i < screenLimit; i++){
	  if((i + shift < d.numOfMonsters) && (i + shift >= 0)){
	    char currMon = d.monsters[i + shift].representation;
	    int yDifference = d.pc.location.row - d.monsters[i + shift].location.row;
	    int xDifference = d.pc.location.col - d.monsters[i + shift].location.col;
	    char first[6];
	    char second[5];
	    char status[6];
	    if(yDifference < 0){
	      yDifference = yDifference * -1;
	      strcpy(first, "south");
	    }
	    else{
	      strcpy(first, "north");
	    }
	    if(xDifference < 0){
	      xDifference = xDifference * -1;
	      strcpy(second, "east");
	    }
	    else{
	      strcpy(second, "west");
	    }
	    if(d.monsters[i + shift].alive == 0){
	      strcpy(status, "alive");
	    }
	    else{
	      strcpy(status, "dead");
	    }
	    //print
	    //put all of the strings together/
	    mvprintw(i + 1, DUNGEON_WIDTH / 5, "Monster: %c is ", currMon);
	    printw("%d ", yDifference);
	    printw("%s and ", first);
	    printw("%d ", xDifference);
	    printw("%s, Status: %s.\n", second, status);
	  }	
	}
	int32_t currCommand = getch();
	switch(currCommand){
	case KEY_UP:
	  if(shift != 0){
	    shift--;
	  }
	  break;
	case KEY_DOWN:
	  if(shift + screenLimit < d.numOfMonsters){
	    shift++;
	  }
	  break;
	case 27:
	  inMonsterScreen = 1;
	  break;
	default: 
	  break;
	}
	refresh();
      } 
    }
    else if(ch == 'Q'){
      break;
    }else{
      d.runs++;
      runGame(&d, ch); 
    }

  }

  endwin();
  printf("GAME OVER\n");
  if(d.monsters[0].alive == 1){
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

void action(char action, int *addRow, int *addCol){
  switch(action){
    //move upper left
  case '7':
  case 'y':
    *addRow = -1;
    *addCol = -1;
    break;
    //move up
  case '8':
  case 'k':
    *addRow = -1;
    *addCol = 0;
    break;
    //move upper right
  case '9':
  case 'u':
    *addRow = -1;
    *addCol = 1;
    break;
    //move right
  case '6':
  case 'l':
    *addRow = 0;
    *addCol = 1;
    break;
    //lower right
  case '3':
  case 'n':
    *addRow = 1;
    *addCol = 1;
    break;
    //move down
  case '2':
  case 'j':
    *addRow = 1;
    *addCol = 0;
    break;
    //lower left
  case '1':
  case 'b':
    *addRow = 1;
    *addCol = -1;
    break;
    //left
  case '4':
  case 'h':
    *addRow = 0;
    *addCol = -1;
    break;
    //down stairs
    //rest
  case '5':
    break;
  default:
    break;
  }
}

void runGame(Dungeon *d, char userInput){
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
      int addRow = 0;
      int addCol = 0;
      action(userInput, &addRow, &addCol);
      if((d->pc.location.row + addRow > 0 && d->pc.location.row + addRow < 20) && 
	 (d->pc.location.col + addCol > 0 && d->pc.location.col + addCol < 79)){
	if(d->hardnessMap[d->pc.location.row + addRow][d->pc.location.col + addCol] == MIN_HARDNESS){
	  moveCharacter(d, d->pc.location.row + addRow, d->pc.location.col + addCol, &d->pc);	  
	}
      }
      newPrintMap(d);
      generateNormalPathMap(d);
      generateTunnelPathMap(d);
    }else{
	moveMonster(d, curr);
    }
  }
  heap_delete(&h);
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

void newPrintMap(Dungeon *d){
  clear();
  int i, j;
  for(i = 0; i < DUNGEON_HEIGHT; i++){
    for(j = 0; j < DUNGEON_WIDTH; j++){
      char temp;
      if(d->characterMap[i][j] != '\0'){
	temp = d->characterMap[i][j];
      }else{
	temp = d->map[i][j];
      }
      mvaddch(i, j, temp);
    }
  }
  refresh();
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
