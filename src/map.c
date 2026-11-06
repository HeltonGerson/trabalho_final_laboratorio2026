#include "rogue.h"

MapCell **createMap(void) {
  MapCell **m = malloc(sizeof(MapCell *) * MAP_SIZE);
  for (int i = 0; i < MAP_SIZE; ++i) {
    m[i] = calloc(MAP_SIZE, sizeof(MapCell));
  }
  return m;
}

void setupFloor(void) {
  // Layout em cruz: 5 salas no 1º andar
  // Centro: sala inicial
  map[1][1].id = 1;
  map[1][1].tiles = readRoom(1);

  // salas conectadas
  map[0][1].id = 2;
  map[0][1].tiles = readRoom(2);

  map[2][1].id = 2;
  map[2][1].tiles = readRoom(2);

  map[1][0].id = 2;
  map[1][0].tiles = readRoom(2);

  map[1][2].id = 2;
  map[1][2].tiles = readRoom(2);

  updateFogOfWar();
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
