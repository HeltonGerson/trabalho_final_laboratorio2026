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
    room = readRoom(1);
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
        case KEY_UP:
        case 'w':
        case 'k':
            newPos.y--;
            break;
        case KEY_DOWN:
        case 's':
        case 'j':
            newPos.y++;
            break;
        case KEY_LEFT:
        case 'a':
        case 'h':
            newPos.x--;
            break;
        case KEY_RIGHT:
        case 'd':
        case 'l':
            newPos.x++;
            break;
    }
    movePlayer(newPos);
}

void movePlayer(Position newPos)
{
    int y = newPos.y, x = newPos.x;
    if (x >= ROOM_WIDTH || x < 0 || y >= ROOM_HEIGHT || y < 0)
        return;
    
    mvaddch(0,0, room[y][x].ch);
    refresh();
    if (room[newPos.y][newPos.x].ch == ACS_UARROW)
    {
        room = readRoom(2);
    }
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
