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
    char ch;
} Entity;

typedef struct
{
    int ch;
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
// funcoes do mapa
Tile** readRoom(int n);
void     freeRoom();
// funcoes do player
Entity* createPlayer(Position start_pos);
void    handleInput (int ch);
void    movePlayer  (Position newPos);
// funcoes do draw
void drawRoom();
void drawEntity(Entity* e);
void drawEverything();

// funcoes de tela
WINDOW *create_newwin(int height, int width, int starty, int startx);
/* extern: faz com que o compiliador assuma
 * que as variaveis estejam disponiveis */
extern const int ROOM_HEIGHT;
extern const int ROOM_WIDTH;
extern Entity*   player;
extern Tile**    room;
extern Tile***   room_choices;
extern int**     map;
extern WINDOW*   win;
