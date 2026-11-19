#ifndef ROGUE_H
#define ROGUE_H

#include <dirent.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SAVE_PATH "saves/savegame.dat"

typedef enum
{
    COLOR_PAIR_PLAYER = 1,
    COLOR_PAIR_ENEMY,
    COLOR_PAIR_ARCHER,
    COLOR_PAIR_BOSS,
    COLOR_PAIR_ITEM,
    COLOR_PAIR_SHOT_PLAYER,
    COLOR_PAIR_SHOT_ENEMY,
    COLOR_PAIR_HIT
} ColorOption;

typedef enum
{
    MENU_NEW_GAME,
    MENU_CONTINUE,
    MENU_QUIT
} MenuOption;


// structs base
typedef struct
{
    int x, y;
} Position;

typedef enum
{
    ENTITY_PLAYER,
    ENTITY_ENEMY_MELEE,
    ENTITY_ENEMY_RANGED,
    ENTITY_BOSS,
    ENTITY_ITEM
} EntityType;

typedef struct
{
    Position   pos;
    chtype     ch;
    EntityType type;
    int        hp;
    int        max_hp;
    int        attack;
    int        ranged_attack;
    int        ranged_range;
    int        ranged_cooldown;
    int        defense;
    bool       active;
} Entity;

typedef struct
{
    chtype ch;
    bool   walkable;
} Tile;

typedef struct
{
    int     id;
    Tile**  tiles;
    Entity* entities;
    int     entity_count;
    bool    visited;
    bool    discovered;
} MapCell;

typedef struct s
{
    Position  coord;
    struct s* next;
} coordNode;

typedef struct
{
    coordNode* top;
    int        size;
} coordStack;

typedef struct
{
    coordNode* front;
    coordNode* back;
    int size;
} coordQueue;

typedef struct l
{
    char* msg;
    struct l* next;
} logNode;

typedef struct
{
    logNode* front;
    logNode* back;
    int size;
} logQueue;

// funcoes das structs
// pilha
coordStack* initializeCoordStack();
void        pushCoord(int x, int y, coordStack* stack);
void        popCoord(coordStack* stack);
void        eraseStack(coordStack* st);
// fila
coordQueue* initializeCoordQueue();
void        pushCoordQueue(int x, int y, coordQueue* q);
coordNode   popCoordQueue(coordQueue* q);
void        eraseCoordQueue(coordQueue* q);
logQueue*   initializeLogQueue();
void        pushLogQueue(char* msg, logQueue* q);
char*       popLogQueue(logQueue* q);
void        eraseLogQueue(logQueue* q);

// funcoes da engine
void       cursesSetup(void);
bool       saveExists(void);
MenuOption showMainMenu(void);
bool       gameLoop(void);
bool       showGameOver(void);
void       cleanupRun(void);
void       closeGame(void);

// funcoes das rooms
Tile** readRoom(int room_type);
void   freeRoomTiles(Tile** tiles);
void   roomDoors(void);
int    setRoom(int x, int y, int choosed_room);
int    countTxtFiles(const char* dir_path);

// funcoes de entidades e player
void    initEntity(Entity* e, EntityType type, int x, int y);
int     spawnRoomEntities(MapCell* cell);
void    freeRoomEntities(MapCell* cell);
Entity* getEntityAt(int x, int y);
void    dealDamage(int raw_attack, Entity* target);
void    attackEntity(Entity* attacker, Entity* target);
bool    canHitLineOfSight(Position origin, Position target, int range, bool allow_diag, int* out_dx,
                          int* out_dy);
bool    fireRangedAttack(Entity* attacker, int dx, int dy);
Entity* createPlayer(Position start_pos);
void    handleInput(int ch);
void    movePlayer(Position newPos);
void    chasePlayer(Entity* e);
void    actRangedEnemy(Entity* e);
void    evadePlayer(Entity* e);
void    actBoss(Entity* e);
void    chooseItem(Entity* e);

// funcoes do draw
void drawRoom(void);
void drawRoomEntities(void);
void drawMinimap(void);
void drawHud(void);
void drawEntity(Entity* e);
void drawHitFlash(Position pos);
void drawProjectilePath(Position from, Position to, int dx, int dy, bool is_player,
                        bool hit_target);
void drawEverything(void);
void drawLogs();

// funcoes do map
MapCell** createMap(void);
void      setupFloor(int start_pos_x, int start_pos_y, int number_of_rooms);
void      updateFogOfWar(void);
void      freeMap(void);

// funcoes de tela
WINDOW* create_newwin(int height, int width, int starty, int startx);
void    layoutWindows(void);
void    destroyWindows(void);

// tempo
void passTime(void);

// logs
void logDefault(const char* msg);


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
extern WINDOW*   win_hud;
extern Position* top;
extern logQueue* logs;

#endif
