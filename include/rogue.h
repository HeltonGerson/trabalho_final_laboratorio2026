#ifndef ROGUE_H
#define ROGUE_H

#include <ncurses.h>
#include <stdlib.h>

// structs base
typedef struct
{
    int y;
    int x;
} Position;

typedef struct
{
    Position pos;
    chtype   ch;
} Entity;

typedef struct
{
    chtype ch;
    bool   walkable;
} Tile;

typedef struct
{
    int    id;
    Tile** tiles;
    bool   visited;
    bool   discovered;
} MapCell;

// funcoes da engine
void cursesSetup(void);
void gameLoop(void);
void closeGame(void);

// funcoes das rooms
Tile** readRoom(int n);
void   freeRoomTiles(Tile** tiles);
void   roomDoors(void);

// funcoes do player
Entity* createPlayer(Position start_pos);
void    handleInput(int ch);
void    movePlayer(Position newPos);

// funcoes do draw
void drawRoom(void);
void drawMinimap(void);
void drawEntity(Entity* e);
void drawEverything(void);

// funcoes do map
MapCell** createMap(void);
void      setupFloor(void);
void      updateFogOfWar(void);
void      freeMap(void);

// funcoes de tela
WINDOW* create_newwin(int height, int width, int starty, int startx);

/* extern: variaveis globais compartilhadas */
extern const int ROOM_HEIGHT;
extern const int ROOM_WIDTH;
extern const int MAP_SIZE;
extern Entity*   player;
extern Tile**    curr_room;
extern MapCell** map;
extern int       map_y;
extern int       map_x;
extern WINDOW*   win;
extern WINDOW*   win_minimapa;

#endif
