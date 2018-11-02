#include <cstdlib>
#include <iostream>
#include <string>

#include "dice.h"

dice::dice(): base(0), numberOfDice(0), sidesOfDice(0)
{}

dice::dice(int8_t offset, int8_t numOfDice, int8_t numOfSides):
base(offset),
  numberOfDice(numOfDice),
  sidesOfDice(numOfSides)
{}

void dice::setBase(int8_t offset){
  base = offset;
}

int8_t dice::getBase(){
  return base;
}

void dice::setNumberOfDice(int8_t numOfDice){
  numberOfDice = numOfDice;
}
int8_t dice::getNumberOfDice(){
  return numberOfDice;
}

void dice::setSidesOfDice(int8_t numOfSides){
  sidesOfDice = numOfSides;
}

int8_t dice::getSidesOfDice(){
  return sidesOfDice;
}

std::string dice::getString(){
  std::string result = "";
  result+= (char) this->base;
  result+= "+";
  result+= (char) this->numberOfDice;
  result+= "d";
  result+= (char) this->sidesOfDice;
  return result;
}

//validate number for costructor?
bool isValid(int8_t offset, int8_t numOfDice, int8_t numOfSides){
  if(offset < 0 && numOfDice < 0 && numOfSides < 0){
    return false;
  }
  return true;
}
