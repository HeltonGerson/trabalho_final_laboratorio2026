#ifndef ROGUE_H
#define ROGUE_H

#include <dirent.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// structs base
typedef struct
{
    int x, y;
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

typedef struct s
{
    Position  coord;
    struct s* below;
} coordNode;

typedef struct
{
    coordNode* top;
    int        size;
} coordStack;

// funcoes das structs
coordStack* initializeCoordStack();
void        pushCoord(int x, int y, coordStack* stack);
void        popCoord(coordStack* stack);
void        eraseStack(coordStack* st);

// funcoes da engine
void cursesSetup(void);
void gameLoop(void);
void closeGame(void);

// funcoes das rooms
Tile** readRoom(int room_type);
void   freeRoomTiles(Tile** tiles);
void   roomDoors(void);
int    setRoom(int x, int y, int choosed_room);
int    countTxtFiles(const char* dir_path);

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
void      setupFloor(int start_pos_x, int start_pos_y, int number_of_rooms);
void      updateFogOfWar(void);
void      freeMap(void);

// funcoes de tela
WINDOW* create_newwin(int height, int width, int starty, int startx);
void    layoutWindows(void);
void    destroyWindows(void);

/* extern: variaveis globais compartilhadas */
extern const int ROOM_HEIGHT;
extern const int ROOM_WIDTH;
extern const int MAP_SIZE;
extern const int MAP_HEIGHT;
extern const int MAP_WIDTH;
extern Entity*   player;
extern Tile**    curr_room;
extern MapCell** map;
extern int       map_y;
extern int       map_x;
extern WINDOW*   win;
extern WINDOW*   win_minimapa;
extern Position* top;

#endif
