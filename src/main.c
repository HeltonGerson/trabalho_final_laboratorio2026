#include "rogue.h"

// declaracoes globais
Entity* player;
Tile**  map;

const int MAP_WIDTH  = 40;
const int MAP_HEIGHT = 16;

int main()
{
    cursesSetup();
    map    = createMapTiles();
    Position start_pos = setupMap();
    player = createPlayer(start_pos);

    gameLoop();

    closeGame();
    return 0;
}


Entity* createPlayer(Position start_pos)
{
    Entity* newPlayer = malloc(sizeof(Entity));

    newPlayer->pos.y = start_pos.y;
    newPlayer->pos.x = start_pos.x;
    newPlayer->ch = '@';

    return newPlayer;
}

void handleInput(int input)
{
    Position newPos = player->pos;
    switch(input)
    {
        case 'k':
            newPos.y--;
            break;
        case 'j':
            newPos.y++;
            break;
        case 'h':
            newPos.x--;
            break;
        case 'l':
            newPos.x++;
            break;
    }
    movePlayer(newPos);
}

void movePlayer(Position newPos)
{
    if (map[newPos.y][newPos.x].walkable)
    {
        player->pos.y = newPos.y;
        player->pos.x = newPos.x;
    }
}
