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
    char ch;
    bool walkable;
} Tile;

// funcoes da engine
void cursesSetup();
void gameLoop();
void closeGame();
// funcoes do mapa
Tile**   createMapTiles();
void     freeMap();
Position setupMap();
// funcoes do player
Entity* createPlayer(Position start_pos);
void    handleInput (int ch);
void    movePlayer  (Position newPos);
// funcoes do draw
void drawMap();
void drawEntity(Entity* e);
void drawEverything();

/* extern: faz com que o compiliador assuma
 * que as variaveis estejam disponiveis */
extern const int MAP_HEIGHT;
extern const int MAP_WIDTH;
extern Entity*   player;
extern Tile**    map;
