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
    chtype ch;
} Entity;

typedef struct
{
    chtype ch;
    bool walkable;
} Tile;

typedef struct
{
    int id;
    //int width;
    //int height;
    Tile** tiles;
} Room;

// funcoes da engine
void cursesSetup();
void gameLoop();
void closeGame();
// funcoes das rooms
Tile** readRoom(int n);
void   freeRoom();
void   roomDoors();
Tile*** LoadRooms();
// funcoes do player
Entity* createPlayer(Position start_pos);
void    handleInput (int ch);
void    movePlayer  (Position newPos);
// funcoes do draw
void drawRoom();
void drawEntity(Entity* e);
void drawEverything();
// funcoes do map
int** createMap();
void  fillMap();
void  freeMap();

// funcoes de tela
WINDOW *create_newwin(int height, int width, int starty, int startx);
/* extern: faz com que o compiliador assuma
 * que as variaveis estejam disponiveis */
extern const int ROOM_HEIGHT;
extern const int ROOM_WIDTH;
extern const int MAP_SIZE;
extern const int N_ROOMS;
extern Entity*   player;
extern Tile**    curr_room;
extern Tile***   room_choices;
extern int**     map;
extern int       map_y;
extern int       map_x;
extern WINDOW*   win;
