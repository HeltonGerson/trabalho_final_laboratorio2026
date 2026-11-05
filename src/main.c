#include "rogue.h"

// declaracoes globais
Entity* player;
Tile**  map;
WINDOW* win;

const int MAP_WIDTH  = 50;
const int MAP_HEIGHT = 15;

int main()
{
    cursesSetup();
    win    = create_newwin(MAP_HEIGHT, MAP_WIDTH);
    map    = createMapTiles();
    Position start_pos = setupMap();
    player = createPlayer(start_pos);

    refresh();

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

WINDOW *create_newwin(int height, int width)
{
    WINDOW *local_win;
    //local_win = newwin(height, width, 0, 0);
    local_win = newwin(height, width, (LINES - height) / 2, (COLS - width) / 2);
    wrefresh(local_win);
    return local_win;
}
