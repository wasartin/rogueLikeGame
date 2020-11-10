#include <iostream>
#include <cstdlib>

#include "utils.h"


int generateRange(int min, int max){
  return rand() % (max - min) + min;
}
