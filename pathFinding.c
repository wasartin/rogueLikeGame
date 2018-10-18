#include <stdio.h>
#include <limits.h>

#include "pathFinding.h"
#include "dungeonInfo.h"

static int32_t comparator(const void *key, const void *with) {
  return ((dist_t *) key)->cost - ((dist_t *) with)->cost;
}

void generateNormalPathMap(Dungeon *d){
  //make heap
  heap_t h;
  uint32_t x, y;
  static dist_t distance[DUNGEON_HEIGHT][DUNGEON_WIDTH], *u;
  static int initilized = 0;

  if(initilized == 0){
    for(y = 0; y < DUNGEON_HEIGHT; y++){
      for(x = 0; x < DUNGEON_WIDTH; x++){
	distance[y][x].pos[1] = y;
	distance[y][x].pos[0] = x;
      }
    }
    initilized = 1;
  } 
  for(y = 0; y < DUNGEON_HEIGHT; y++){
    for(x = 0; x < DUNGEON_WIDTH; x++){
      distance[y][x].pos[dim_y] = y;
      distance[y][x].pos[dim_x] = x;
    }
  }
  
  //mark all of the dungeon at inifinty
  for(y = 0; y < DUNGEON_HEIGHT; y++){
    for(x = 0; x < DUNGEON_WIDTH; x++){
      if(y == d->pc.location.row && x == d->pc.location.col){
	distance[y][x].cost = 0;
      }else{
	distance[y][x].cost = INT_MAX;
      }
    }
  }
  
  //init
  heap_init(&h, comparator, NULL);
  
  //heap insert
  for(y = 0; y < DUNGEON_HEIGHT; y++){
    for(x = 0; x < DUNGEON_WIDTH; x++){
      if(d->hardnessMap[y][x] == MIN_HARDNESS){
	distance[y][x].hn = heap_insert(&h, &distance[y][x]);
      }
      else{
	distance[y][x].hn = NULL;
      }
    }
  }

  while((u = heap_remove_min(&h))){
    u->hn = NULL;
    
    //check if it is in the heap AND if the spot is less than the current.
    if((distance[u->pos[dim_y] - 1][u->pos[dim_x] - 1].hn) && 
       (distance[u->pos[dim_y] - 1][u->pos[dim_x] - 1].cost >  u->cost + 1)) {
      distance[u->pos[dim_y] - 1][u->pos[dim_x] - 1].cost = u->cost + 1;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y] - 1][u->pos[dim_x] - 1].hn);
    }
    if((distance[u->pos[dim_y] - 1][u->pos[dim_x]   ].hn) && 
       (distance[u->pos[dim_y] - 1][u->pos[dim_x]   ].cost > u->cost + 1)){
      distance[u->pos[dim_y] - 1][u->pos[dim_x]   ].cost =  u->cost + 1;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y] - 1][u->pos[dim_x]   ].hn);
    }
    if((distance[u->pos[dim_y] - 1][u->pos[dim_x] + 1].hn) &&
       (distance[u->pos[dim_y] - 1][u->pos[dim_x] + 1].cost > u->cost + 1)){
      distance[u->pos[dim_y] - 1][u->pos[dim_x] + 1].cost = u->cost + 1;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y] - 1][u->pos[dim_x] + 1].hn);
    }
    if((distance[u->pos[dim_y]   ][u->pos[dim_x] - 1].hn) &&
       (distance[u->pos[dim_y]   ][u->pos[dim_x] - 1].cost > u->cost + 1)){
      distance[u->pos[dim_y]   ][u->pos[dim_x] - 1].cost = u->cost + 1;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y]   ][u->pos[dim_x] - 1].hn);
    }
    if((distance[u->pos[dim_y]   ][u->pos[dim_x] + 1].hn) &&
       (distance[u->pos[dim_y]   ][u->pos[dim_x] + 1].cost > u->cost + 1)){
      distance[u->pos[dim_y]   ][u->pos[dim_x] + 1].cost = u->cost + 1;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y]   ][u->pos[dim_x] + 1].hn);
    }
    if((distance[u->pos[dim_y] + 1][u->pos[dim_x] - 1].hn) && 
       (distance[u->pos[dim_y] + 1][u->pos[dim_x] - 1].cost > u->cost + 1)){
      distance[u->pos[dim_y] + 1][u->pos[dim_x] - 1].cost = u->cost + 1;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y] + 1][u->pos[dim_x] - 1].hn);
    }
    if((distance[u->pos[dim_y] + 1][u->pos[dim_x]   ].hn) &&
       (distance[u->pos[dim_y] + 1][u->pos[dim_x]   ].cost > u->cost + 1)){
      distance[u->pos[dim_y] + 1][u->pos[dim_x]   ].cost = u->cost + 1;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y] + 1][u->pos[dim_x]   ].hn);
    }
    if((distance[u->pos[dim_y] + 1][u->pos[dim_x] + 1].hn) && 
       (distance[u->pos[dim_y] + 1][u->pos[dim_x] + 1].cost > u->cost  + 1)){
      distance[u->pos[dim_y] + 1][u->pos[dim_x] + 1].cost = u->cost + 1;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y] + 1][u->pos[dim_x] + 1].hn);
      }
    
  }
  for(y = 0; y < DUNGEON_HEIGHT; y++){
    for(x = 0; x < DUNGEON_WIDTH; x++){
	d->nonTunnelPaths[y][x] = distance[y][x].cost;
    }
  }
  heap_delete(&h);

}

void generateTunnelPathMap(Dungeon *d){
    //make heap
  heap_t h;
  uint32_t x, y;
  static dist_t distance[DUNGEON_HEIGHT][DUNGEON_WIDTH], *u;
  static int initilized = 0;

  //since this will run multiply time, might as well save some time and see if it has been init yet.
  if(initilized == 0){
    for(y = 0; y < DUNGEON_HEIGHT; y++){
      for(x = 0; x < DUNGEON_WIDTH; x++){
	distance[y][x].pos[1] = y;
	distance[y][x].pos[0] = x;
      }
    }
    initilized = 1;
  }

  //mark all of the dungeon at inifinty
  for(y = 0; y < DUNGEON_HEIGHT; y++){
    for(x = 0; x < DUNGEON_WIDTH; x++){
      if(y == d->pc.location.row && x == d->pc.location.col){
	distance[y][x].cost = 0;
      }else{
	distance[y][x].cost = INT_MAX;
      }
    }
  }

  //init
  heap_init(&h, comparator, NULL);
  
  //heap insert
  for(y = 0; y < DUNGEON_HEIGHT; y++){
    for(x = 0; x < DUNGEON_WIDTH; x++){
      if(d->hardnessMap[y][x] != MAX_HARDNESS){
	distance[y][x].hn = heap_insert(&h, &distance[y][x]);
      }
      else{
	distance[y][x].hn = NULL;
      }
    }
  }

  while((u = heap_remove_min(&h))){
    u->hn = NULL;
    int32_t addlCost = (d->hardnessMap[u->pos[dim_y]][u->pos[dim_x]] / 85) + 1;
    //  /*
    //check if it is in the heap AND if the spot is less than the current.
    if((distance[u->pos[dim_y] - 1][u->pos[dim_x] - 1].hn) && 
       (distance[u->pos[dim_y] - 1][u->pos[dim_x] - 1].cost >  u->cost + addlCost)) {
      distance[u->pos[dim_y] - 1][u->pos[dim_x] - 1].cost = u->cost + addlCost;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y] - 1][u->pos[dim_x] - 1].hn);
    }
    if((distance[u->pos[dim_y] - 1][u->pos[dim_x]   ].hn) && 
       (distance[u->pos[dim_y] - 1][u->pos[dim_x]   ].cost > u->cost + addlCost)){
      distance[u->pos[dim_y] - 1][u->pos[dim_x]   ].cost =  u->cost + addlCost;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y] - 1][u->pos[dim_x]   ].hn);
    }
    if((distance[u->pos[dim_y] - 1][u->pos[dim_x] + 1].hn) &&
       (distance[u->pos[dim_y] - 1][u->pos[dim_x] + 1].cost > u->cost + addlCost)){
      distance[u->pos[dim_y] - 1][u->pos[dim_x] + 1].cost = u->cost + addlCost;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y] - 1][u->pos[dim_x] + 1].hn);
    }
    if((distance[u->pos[dim_y]   ][u->pos[dim_x] - 1].hn) &&
       (distance[u->pos[dim_y]   ][u->pos[dim_x] - 1].cost > u->cost + addlCost)){
      distance[u->pos[dim_y]   ][u->pos[dim_x] - 1].cost = u->cost + addlCost;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y]   ][u->pos[dim_x] - 1].hn);
    }
    if((distance[u->pos[dim_y]   ][u->pos[dim_x] + 1].hn) &&
       (distance[u->pos[dim_y]   ][u->pos[dim_x] + 1].cost > u->cost + addlCost)){
      distance[u->pos[dim_y]   ][u->pos[dim_x] + 1].cost = u->cost + addlCost;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y]   ][u->pos[dim_x] + 1].hn);
    }
    if((distance[u->pos[dim_y] + 1][u->pos[dim_x] - 1].hn) && 
       (distance[u->pos[dim_y] + 1][u->pos[dim_x] - 1].cost > u->cost + addlCost)){
      distance[u->pos[dim_y] + 1][u->pos[dim_x] - 1].cost = u->cost + addlCost;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y] + 1][u->pos[dim_x] - 1].hn);
    }
    if((distance[u->pos[dim_y] + 1][u->pos[dim_x]   ].hn) &&
       (distance[u->pos[dim_y] + 1][u->pos[dim_x]   ].cost > u->cost + addlCost)){
      distance[u->pos[dim_y] + 1][u->pos[dim_x]   ].cost = u->cost + addlCost;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y] + 1][u->pos[dim_x]   ].hn);
    }

    if((distance[u->pos[dim_y] + 1][u->pos[dim_x] + 1].hn) && 
       (distance[u->pos[dim_y] + 1][u->pos[dim_x] + 1].cost > u->cost + addlCost)){
      distance[u->pos[dim_y] + 1][u->pos[dim_x] + 1].cost = u->cost + addlCost;
      heap_decrease_key_no_replace(&h, distance[u->pos[dim_y] + 1][u->pos[dim_x] + 1].hn);
      }
  }

  for(y = 0; y < DUNGEON_HEIGHT; y++){
    for(x = 0; x < DUNGEON_WIDTH; x++){
	d->tunnelPaths[y][x] = distance[y][x].cost;
    }
  }
  heap_delete(&h);
}
