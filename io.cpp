#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>
#include <cstring>

#include "io.h"
#include "dungeonInfo.h"
#include "buildDungeon.h"
#include "character.h"
#include "move.h"

void initTerminal(){
  initscr();
  raw();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  
}

void resetTerminal(){
  endwin();
  
}

void handleUserInput(Dungeon *d){
  int32_t action = getch();
  uint8_t targetRow = d->pc.location.row;
  uint8_t targetCol = d->pc.location.col;
  switch(action){
    //move upper left 7, y
  case 55:
  case 121:
    targetRow += -1;
    targetCol += -1;
    if(checkTargetCoordinate(d, targetRow, targetCol)){
      moveCharacter(d, targetRow, targetCol, &d->pc);
    }
    break;
    //move up 8, k, key up
  case 56: 
  case 107:
  case KEY_UP:
    targetRow += -1;
    if(checkTargetCoordinate(d, targetRow, targetCol)){
      moveCharacter(d, targetRow, targetCol, &d->pc);
    }
    break;
    //move upper right 9, u
  case 57:
  case 117:
    targetRow += -1;
    targetCol += 1;
    if(checkTargetCoordinate(d, targetRow, targetCol)){
      moveCharacter(d, targetRow, targetCol, &d->pc);
    }
    //move char
    break;
  case 54:
  case KEY_RIGHT:
  case 108:
    targetCol += 1;
    if(checkTargetCoordinate(d, targetRow, targetCol)){
      moveCharacter(d, targetRow, targetCol, &d->pc);
    }
    break;
    //lower right 3, n
  case 51:
  case 110:
    targetRow += 1;
    targetCol += 1;
    if(checkTargetCoordinate(d, targetRow, targetCol)){
      moveCharacter(d, targetRow, targetCol, &d->pc);
    }
    break;
    //move down 2, 
  case 50:
  case KEY_DOWN:
  case 106:
    targetRow += 1;
    if(checkTargetCoordinate(d, targetRow, targetCol)){
      moveCharacter(d, targetRow, targetCol, &d->pc);
    }
    break;
    //lower left 1, b
  case 49:
  case 98:
    targetRow += 1;
    targetCol += -1;
    if(checkTargetCoordinate(d, targetRow, targetCol)){
      moveCharacter(d, targetRow, targetCol, &d->pc);
    }
    break;
    //left 4, Keyleft, h
  case 52:
  case KEY_LEFT:
  case 104:
    targetCol += -1;
    if(checkTargetCoordinate(d, targetRow, targetCol)){
      moveCharacter(d, targetRow, targetCol, &d->pc);
    }
    break;
    //down stairs
    //rest //5
  case 53:
    break;
  case 60://<
    if(d->map[d->pc.location.row][d->pc.location.col] == '<'){
      //new dungeon
      free(d->rooms);
      free(d->monsters);
      moveLevel(d);
    }
    break;
  case 62://>
    if(d->map[d->pc.location.row][d->pc.location.col] == '>'){
      //make new dungeon
      free(d->rooms);
      free(d->monsters);
      moveLevel(d);
    }
    break;
  case 103:// g
    d->fogOfWar = false;
    teleportPlayer(d);
    break;
  case 109://m
    displayMonsterList(d);
    break;
  case 102://toggle fog of war
    d->fogOfWar = !d->fogOfWar;
    break;
  case 81: //Q
    d->pc.alive = false;
    break;
  default:
    break;
  }
}

void displayMonsterList(Dungeon *d){
  uint8_t i;
  uint8_t screenLimit = 10;
  if(d->numOfMonsters < 10){
    screenLimit = d->numOfMonsters;
  }
  uint8_t shift = 0;
  bool inMonsterScreen = true;
  clear();
  while(inMonsterScreen == true){
    for(i = 0; i < screenLimit; i++){
      if((i + shift < d->numOfMonsters) && (i + shift >= 0)){
	char currMon = d->monsters[i + shift].type;
	int yDifference = d->pc.location.row - d->monsters[i + shift].location.row;
	int xDifference = d->pc.location.col - d->monsters[i + shift].location.col;
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
	if(d->monsters[i + shift].alive == true){
	  strcpy(status, "alive");
	}
	else{
	  strcpy(status, "dead");
	}
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
      if(shift + screenLimit < d->numOfMonsters){
	shift++;
      }
      break;
    case 27:
      inMonsterScreen = false;
      break;
    default: 
      break;
    }
    refresh();
  } 
}

void teleportPlayer(Dungeon *d){
  bool inTheEtherealRealm = true;
  Character eyeOfKilrogg;
  eyeOfKilrogg.alive = true;
  eyeOfKilrogg.type = '*';
  eyeOfKilrogg.location.row = d->pc.location.row;
  eyeOfKilrogg.location.col = d->pc.location.col;
  while(inTheEtherealRealm){
    uint32_t userInput = getch();
    //ALl movement will be with the * char
    uint8_t targetRow = eyeOfKilrogg.location.row;
    uint8_t targetCol = eyeOfKilrogg.location.col;
    newPrintMap(d);
    switch(userInput){
    case 52://left
    case KEY_LEFT:
    case 104:
      targetCol += -1;
      if(checkTargetCoordinateTeleport(d, targetRow, targetCol)){
        moveCharacter(d, targetRow, targetCol, &eyeOfKilrogg);
      }
      break;
    case 54: //right
    case 108:
    case KEY_RIGHT:
      targetCol += 1;
      if(checkTargetCoordinateTeleport(d, targetRow, targetCol)){
	moveCharacter(d, targetRow, targetCol, &eyeOfKilrogg);
      }
      break;
    case 56: //up
    case 107:
    case KEY_UP:
      targetRow += -1;
      if(checkTargetCoordinateTeleport(d, targetRow, targetCol)){
	moveCharacter(d, targetRow, targetCol, &eyeOfKilrogg);
      }
      break;
    case 50: //down
    case 106:
    case KEY_DOWN:
      targetRow += 1;
      if(checkTargetCoordinateTeleport(d, targetRow, targetCol)){
	moveCharacter(d, targetRow, targetCol, &eyeOfKilrogg);
      }
      break;
    case 55: //top left
    case 121:
      targetRow += -1;
      targetCol += -1;
      if(checkTargetCoordinateTeleport(d, targetRow, targetCol)){
        moveCharacter(d, targetRow, targetCol, &eyeOfKilrogg);
      }
      break;
    case 57: //top right
    case 117:
      targetRow += -1;
      targetCol += 1;
      if(checkTargetCoordinateTeleport(d, targetRow, targetCol)){
        moveCharacter(d, targetRow, targetCol, &eyeOfKilrogg);
      }
      break;
    case 49: //bottom left
    case 98:
      targetRow += 1;
      targetCol += -1;
      if(checkTargetCoordinateTeleport(d, targetRow, targetCol)){
        moveCharacter(d, targetRow, targetCol, &eyeOfKilrogg);
      }
      break;
    case 51: //bottom right
    case 110:
      targetRow += 1;
      targetRow += 1;
      if(checkTargetCoordinateTeleport(d, targetRow, targetCol)){
        moveCharacter(d, targetRow, targetCol, &eyeOfKilrogg);
      }
      break;
    case 114: //r, random move //hot damn, i did well here
      targetRow = generateRange(1, DUNGEON_HEIGHT - 1);
      targetCol  = generateRange(1, DUNGEON_WIDTH - 1);
    case 103: //g, place where eyeOfKilrogg is
      inTheEtherealRealm = false;
      eyeOfKilrogg.alive = false;
      d->characterMap[eyeOfKilrogg.location.row][eyeOfKilrogg.location.col] = '\0';
      if(d->hardnessMap[d->pc.location.row][d->pc.location.col] != MIN_HARDNESS){
	d->map[d->pc.location.row][d->pc.location.col] = '#';
	d->hardnessMap[d->pc.location.row][d->pc.location.col] = MIN_HARDNESS;
      }
      moveCharacter(d, targetRow, targetCol, &d->pc);
      d->fogOfWar = true;
      break;//Should probably add Q, so the player can always Quit
    default:
      break;
    }
  }
}

void moveLevel(Dungeon *d){
    createDungeon(d);
    uint8_t row = d->rooms[0].topLeftCoord.row;
    uint8_t col = d->rooms[0].topLeftCoord.col;
    d->pc.alive = true;
    d->pc.type = '@';
    d->pc.location.row = row;
    d->pc.location.col = col;
    d->pc.speed = generateRange(5, 21);
    d->pc.turn = 1000/ d->pc.speed;
    d->characterMap[row][col] = '@';
    moveCharacter(d, row, col, &d->pc);
    d->monsters = (Character*)malloc(d->numOfMonsters * sizeof(Character));
    makeMonsters(d);
    placeMonsters(d);
}

void newPrintMap(Dungeon *d){
  if(d->fogOfWar == true){
    printFog(d);
  }else {
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
    mvprintw(21, 1, "Player is at row: %d, col: %d", d->pc.location.row, d->pc.location.col);
    refresh();
  } 
}

void printFog(Dungeon *d){
  clear();
  for(int i = 0; i < DUNGEON_HEIGHT; i++){
    for(int j = 0; j < DUNGEON_WIDTH; j++){
      char temp;
      if(withinSight(d, i, j) == true){
	if(d->characterMap[i][j] != '\0'){
	  temp = d->characterMap[i][j];
	}else{
	  temp = d->map[i][j];
	}
      }else {
	temp = ' ';
      }
      mvaddch(i, j, temp);
    }
  }
  mvprintw(21, 1, "Player is at row: %d, col: %d", d->pc.location.row, d->pc.location.col);
  mvprintw(22, 1, "There's something in the mist!");
  refresh();
}

bool withinSight(Dungeon *d, uint8_t row, uint8_t col){
  int rowDifference = abs(d->pc.location.row - row);
  int colDifference = abs(d->pc.location.col - col);
  if((rowDifference < 5) && (colDifference < 5)){
    return true;
  }
  return false;
}


