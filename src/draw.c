#include "rogue.h"

// desenha a room atual
void drawRoom(void)
{
    for (int y = 0; y < ROOM_HEIGHT; ++y)
    {
        for (int x = 0; x < ROOM_WIDTH; ++x)
        {
            mvwaddch(win, y, x, curr_room[y][x].ch);
        }
    }
}

void drawMinimap(void)
{
    if (!win_minimapa || !map)
        return;

    werase(win_minimapa);
    box(win_minimapa, 0, 0);
    mvwprintw(win_minimapa, 0, 11, " MINIMAP ");

    for (int y = 0; y < MAP_HEIGHT; ++y)
    {
        for (int x = 0; x < MAP_WIDTH; ++x)
        {
            int draw_y = 1 + (y * 2);
            int draw_x = 1 + (x * 4);

            if (y == map_y && x == map_x)
            {
                mvwprintw(win_minimapa, draw_y, draw_x, "[@]");
            }
            else if (map[y][x].visited)
            {
                mvwprintw(win_minimapa, draw_y, draw_x, "[X]");
            }
            else if (map[y][x].discovered)
            {
                mvwprintw(win_minimapa, draw_y, draw_x, "[?]");
            }
            else
            {
                mvwprintw(win_minimapa, draw_y, draw_x, "   ");
            }

            if (x + 1 < MAP_WIDTH && map[y][x].id != 0 && map[y][x + 1].id != 0 &&
                (map[y][x].visited || map[y][x + 1].visited))
            {
                mvwaddch(win_minimapa, draw_y, draw_x + 3, '-');
            }

            if (y + 1 < MAP_HEIGHT && map[y][x].id != 0 && map[y + 1][x].id != 0 &&
                (map[y][x].visited || map[y + 1][x].visited))
            {
                mvwaddch(win_minimapa, draw_y + 1, draw_x + 1, '|');
            }
        }
    }
    wrefresh(win_minimapa);
}

void drawEntity(Entity* e) { mvwprintw(win, e->pos.y, e->pos.x, "%c", e->ch); }

void drawEverything(void)
{
    drawRoom();
    drawEntity(player);
    wrefresh(win);
    drawMinimap();
}
