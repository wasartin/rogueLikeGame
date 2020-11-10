#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <endian.h>
#include <inttypes.h>
#include <unistd.h>
#include <ncurses.h>


#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>

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

void parseFile(Dungeon *d);
void printCurrDescription(monsterDesc *curr);
void printMonsterDescriptions(Dungeon *d);
/*
void parseName();
void parseSymb();
void parseColor();
void parseDescription();
void parseSpeed();
void parseDamage();
void parseHP();
void parseAbility();
void parseRarity();
*/

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

  parseFile(&d);
  printMonsterDescriptions(&d);

  exit(0);
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

void clearDescription(monsterDesc *curr){
  curr->name = "";
  curr->desc = "";
  curr->color = "";
  curr->speed = "";
  curr->abil = "";
  curr->hp = "";
  curr->dam = "";
  curr->symb = "";
  curr->rrty = "";
  curr->count = 0;
}

monsterFileType getEnum(std::string const &input){
  if(input == "BEGIN") return BEGIN;
  if(input == "NAME") return NAME;
  if(input == "DESC") return DESC;
  if(input == "COLOR") return COLOR;
  if(input == "SPEED") return SPEED;
  if(input == "ABIL") return ABIL;
  if(input == "HP") return HP;
  if(input == "DAM") return DAM;
  if(input == "SYMB") return SYMB;
  if(input == "RRTY") return RRTY;
  if(input == "END") return END;
  else return UNKNOWN;
}

void parseFile(Dungeon *d){
  std::string fileName;
  fileName = getenv("HOME");
  fileName += "/.rlg327/monster_desc.txt";

  //init ifstream with name we want.
  std::ifstream fileReader(fileName.c_str());
  if(!fileReader.is_open()){
    std::cout << "Could not open file!" << std::endl;
  }

  //get first line and print it
  std::string firstLine;
  getline(fileReader, firstLine);

  std::cout << "File being read: " << firstLine << std::endl;
  bool stillParsing = true;
  monsterDesc curr;
  curr.count = 0;
  while(stillParsing){
    //peek at first line of document
    //swtich case for certain word
    int check = fileReader.peek();
    if(check == -1){
      stillParsing = false;
      break;
    }
    std::string lineId;
    fileReader >> lineId;
    monsterFileType decider = getEnum(lineId);
    std::string temp;
    fileReader.get(); //remove whitespace
    getline(fileReader, temp);
    switch(decider){
    case BEGIN:
      break;
    case NAME:
      curr.name = temp;
      curr.count++;
      break;
    case DESC:
      while(temp != "."){
	curr.desc += temp;
	curr.desc += "\n";
	getline(fileReader, temp);
      }
      curr.count++;
      break;
    case COLOR:
      curr.color = temp;
      curr.count++;
      break;
    case SPEED: //TODO Parse as 3 integers, into dice class maybe?
      curr.speed = temp;
      curr.count++;
      break;    
    case ABIL:
      curr.abil = temp;
      curr.count++;
      break;
    case HP: //TODO Parse as 3 integers, into dice class maybe?
      curr.hp = temp;
      curr.count++;
      break;
    case DAM: //TODO Parse as 3 integers, into dice class maybe?
      curr.dam = temp;
      curr.count++;
      break;
    case SYMB:
      curr.symb = temp;
      curr.count++;
      break;
    case RRTY:
      curr.rrty = temp;
      curr.count++;
      break;
    case END:
      d->monsterDescriptions.push_back(curr);
      clearDescription(&curr);
      break;
    case UNKNOWN:
      std::cout << "Unknown or Default action" << std::endl;
    default:
      std::cout << "";
      clearDescription(&curr);
      break;
    }
  }
  fileReader.close();
}

void printDesc(std::string input){
  for(size_t i = 0; i < input.size(); i++){
    std::cout << input[i];
  }
}

void printCurrDescription(monsterDesc *curr){
  std::cout << curr->name << std::endl;
  //std::cout << curr->desc<< std::endl;
  printDesc(curr->desc);
  std::cout << curr->color << std::endl;
  std::cout << curr->speed << std::endl; //dice
  std::cout << curr->abil << std::endl;
  std::cout << curr->hp << std::endl;  //dice
  std::cout << curr->dam << std::endl; //dice
  std::cout << curr->symb << std::endl;
  std::cout << curr->rrty << std::endl;
}

void printMonsterDescriptions(Dungeon *d){
  std::cout << "Printing monster descriptions" << std::endl;
  for(std::vector<monsterDesc>:: iterator it = d->monsterDescriptions.begin();
      it != d->monsterDescriptions.end(); it++){
    monsterDesc curr = *it;
    printCurrDescription(&curr);
    std::cout << std::endl;
  }
}
/* //teacher recommended having bool check if everything is loading into description
void parseDice(){}
void parseName(){}
void parseSymb(){}
void parseColor(){}
void parseDescription(){}
void parseSpeed(){}
void parseDamage(){}
void parseHP(){}
void parseAbility(){}
void parseRarity(){}
*/

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
