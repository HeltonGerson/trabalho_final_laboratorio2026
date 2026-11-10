#include "rogue.h"
#include <stdlib.h>

MapCell **createMap(void) {
  MapCell **m = malloc(sizeof(MapCell *) * MAP_SIZE);
  for (int i = 0; i < MAP_SIZE; ++i) {
    m[i] = calloc(MAP_SIZE, sizeof(MapCell));
  }
  return m;
}

void setupFloor(int start_pos_x, int start_pos_y, int number_of_rooms) { 
    coordStack *st = initializeCoordStack();
    if(setRoom(start_pos_x, start_pos_y, 1)){
        printw("Erro ao preencher a sala inicial");
        exit(1);
    }
    pushCoord(start_pos_x, start_pos_y, st);
    number_of_rooms--;
    
    int dx[4] = {-1, 0, 1, 0};
    int dy[4] = {0, 1, 0, -1};

    while(number_of_rooms > 0 && st->top != NULL){
       int free_dirs[4];
       int count_dirs = 0;

       for(int i = 0; i < 4; i++){
           int nx = st->top->coord.x + dx[i];
           int ny = st->top->coord.y + dy[i];

           if(nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT && map[nx][ny].tiles == 0){
               free_dirs[count_dirs] = i;
               count_dirs++;
           }
       }

       if(count_dirs == 0){
           popCoord(st);
           continue;
       }
       int choose_dir = free_dirs[rand() % count_dirs];
       int room_type = rand() % 4 + 1;
       int nx = st->top->coord.x + dx[choose_dir];
       int ny = st->top->coord.y + dy[choose_dir];
       if(setRoom(nx, ny, room_type)){
           printw("Erro ao preencher uma sala");
           exit(1);
       }
       pushCoord(nx, ny, st);
       number_of_rooms--;
    }
    updateFogOfWar();
    eraseStack(st);
}

void updateFogOfWar(void) {
  if (!map || map[map_y][map_x].id == 0)
    return;

  map[map_y][map_x].visited = true;
  map[map_y][map_x].discovered = true;

  // Adjacentes conectadas reveladas como ?
  if (map_y > 0 && map[map_y - 1][map_x].id != 0)
    map[map_y - 1][map_x].discovered = true;
  if (map_y + 1 < MAP_SIZE && map[map_y + 1][map_x].id != 0)
    map[map_y + 1][map_x].discovered = true;
  if (map_x > 0 && map[map_y][map_x - 1].id != 0)
    map[map_y][map_x - 1].discovered = true;
  if (map_x + 1 < MAP_SIZE && map[map_y][map_x + 1].id != 0)
    map[map_y][map_x + 1].discovered = true;
}

void freeMap(void) {
  if (!map)
    return;

  for (int y = 0; y < MAP_SIZE; ++y) {
    for (int x = 0; x < MAP_SIZE; ++x) {
      if (map[y][x].tiles) {
        freeRoomTiles(map[y][x].tiles);
        map[y][x].tiles = NULL;
      }
    }
    free(map[y]);
  }
  free(map);
  map = NULL;
}
