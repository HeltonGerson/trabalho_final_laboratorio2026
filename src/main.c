#include "rogue.h"

// declaracoes globais
Entity* player;
Tile**  room;
Tile*** room_choices;
WINDOW* win;

const int ROOM_WIDTH  = 32;
const int ROOM_HEIGHT = 10;
const int N_ROOMS    = 2;

int main()
{
    cursesSetup();
    win  = create_newwin(ROOM_HEIGHT, ROOM_WIDTH, (LINES - ROOM_HEIGHT) / 2, (COLS - ROOM_WIDTH) / 2);
    room = readRoom(2);
    Position start_pos = {5,5};
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
    if (newPos.x >= ROOM_WIDTH-1 || newPos.x < 1 || newPos.y >= ROOM_HEIGHT-1 || newPos.y < 1)
        return;
    else if (room[newPos.y][newPos.x].walkable)
    {
        player->pos.y = newPos.y;
        player->pos.x = newPos.x;
    }
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    wrefresh(local_win);
    return local_win;
}
