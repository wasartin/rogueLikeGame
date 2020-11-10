#ifndef UTILS_H
# define UTILS_H

#include <stdint.h>

typedef struct{
  uint8_t row;
  uint8_t col;
}Coordinate;

int generateRange(int min, int max);

#endif
