#ifndef DICE_H
# define DICE_H

#include <stdint.h>
#include <string>

class dice{
 public:
  int8_t base;
  int8_t numberOfDice;
  int8_t sidesOfDice;
 public:
  dice();
  dice(int8_t offset, int8_t numOfDice, int8_t numOfSides);
  void setBase(int8_t offset);
  int8_t getBase();
  void setNumberOfDice(int8_t numOfDice);
  int8_t getNumberOfDice();
  void setSidesOfDice(int8_t numOfSides);
  int8_t getSidesOfDice();
  std::string getString();
};

void printDice(dice *inputDice);

#endif
