#include "rogue.h"

// desenha a room atual com zoom 2x
void drawRoom(void)
{
    werase(win);
    box(win, 0, 0);

    for (int y = 0; y < ROOM_HEIGHT; ++y)
    {
        for (int x = 0; x < ROOM_WIDTH; ++x)
        {
            chtype ch = curr_room[y][x].ch;

            if (y == 0 && ch == ACS_UARROW)
            {
                mvwaddch(win, 0, x * 2, ch);
                mvwaddch(win, 0, x * 2 + 1, ch);
            }
            else if (y == ROOM_HEIGHT - 1 && ch == ACS_DARROW)
            {
                mvwaddch(win, ROOM_HEIGHT * 2 - 1, x * 2, ch);
                mvwaddch(win, ROOM_HEIGHT * 2 - 1, x * 2 + 1, ch);
            }
            else if (x == 0 && ch == ACS_LARROW)
            {
                mvwaddch(win, y * 2, 0, ch);
                mvwaddch(win, y * 2 + 1, 0, ch);
            }
            else if (x == ROOM_WIDTH - 1 && ch == ACS_RARROW)
            {
                mvwaddch(win, y * 2, ROOM_WIDTH * 2 - 1, ch);
                mvwaddch(win, y * 2 + 1, ROOM_WIDTH * 2 - 1, ch);
            }
            else if (y > 0 && y < ROOM_HEIGHT - 1 && x > 0 && x < ROOM_WIDTH - 1)
            {
                if (ch == '.')
                {
                    mvwaddch(win, y * 2, x * 2, '.');
                }
                else
                {
                    mvwaddch(win, y * 2, x * 2, ch);
                    mvwaddch(win, y * 2, x * 2 + 1, ch);
                    mvwaddch(win, y * 2 + 1, x * 2, ch);
                    mvwaddch(win, y * 2 + 1, x * 2 + 1, ch);
                }
            }
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

void drawEntity(Entity* e) { mvwaddch(win, e->pos.y * 2, e->pos.x * 2, e->ch); }

void drawEverything(void)
{
    drawRoom();
    drawEntity(player);
    wrefresh(win);
    drawMinimap();
}
