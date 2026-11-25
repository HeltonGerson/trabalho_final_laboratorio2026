#include "rogue.h"

/*
 * Converte o id da sala no glifo exibido no minimapa ('S' inicial, 'B' boss,
 * 'I' item ou 'X' sala normal).
 * Chamada por: drawMinimap (src/draw.c).
 */
static char roomGlyph(int id)
{
    int type = id / ROOM_ID_BASE;

    if (type == 0)
        return 'S';
    if (type == 1)
        return 'B';
    if (type == 2)
        return 'I';
    return 'X';
}

/*
 * Desenha todos os tiles da sala atual (curr_room) na janela win em escala 1:1.
 * Chamada por: drawHitFlash, drawProjectilePath e drawEverything (src/draw.c).
 */
void drawRoom(void)
{
    if (!win || !curr_room)
        return;

    for (int y = 0; y < ROOM_HEIGHT; ++y)
    {
        for (int x = 0; x < ROOM_WIDTH; ++x)
        {
            mvwaddch(win, y, x, curr_room[y][x].ch);
        }
    }
}

/*
 * Renderiza a janela win_minimapa como uma janela deslizante 3x3 centrada na sala
 * atual, mostrando salas visitadas, descobertas ([?]), jogador ([@]), bordas
 * fora do grid (###) e conectores ('-' e '|').
 * Chamada por: drawEverything (src/draw.c).
 */
void drawMinimap(void)
{
    if (!win_minimapa || !map)
        return;

    werase(win_minimapa);
    box(win_minimapa, 0, 0);
    mvwprintw(win_minimapa, 0, 3, " MINIMAP ");
    mvwprintw(win_minimapa, 6, 4, " (%d,%d) ", map_x, map_y);

    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            int ny     = map_y + dy;
            int nx     = map_x + dx;
            int draw_y = 1 + (dy + 1) * 2;
            int draw_x = 2 + (dx + 1) * 4;

            if (ny < 0 || ny >= MAP_HEIGHT || nx < 0 || nx >= MAP_WIDTH)
            {
                mvwprintw(win_minimapa, draw_y, draw_x, "###");
                continue;
            }

            if (ny == map_y && nx == map_x)
                mvwprintw(win_minimapa, draw_y, draw_x, "[@]");
            else if (map[ny][nx].visited)
                mvwprintw(win_minimapa, draw_y, draw_x, "[%c]", roomGlyph(map[ny][nx].id));
            else if (map[ny][nx].discovered)
                mvwprintw(win_minimapa, draw_y, draw_x, "[?]");
            else
                mvwprintw(win_minimapa, draw_y, draw_x, "   ");

            if (dx < 1 && nx + 1 < MAP_WIDTH && map[ny][nx].id != 0 && map[ny][nx + 1].id != 0 &&
                (map[ny][nx].visited || map[ny][nx + 1].visited))
            {
                mvwaddch(win_minimapa, draw_y, draw_x + 3, '-');
            }

            if (dy < 1 && ny + 1 < MAP_HEIGHT && map[ny][nx].id != 0 && map[ny + 1][nx].id != 0 &&
                (map[ny][nx].visited || map[ny + 1][nx].visited))
            {
                mvwaddch(win_minimapa, draw_y + 1, draw_x + 1, '|');
            }
        }
    }
}

/*
 * Retorna o atributo de cor e destaque do ncurses correspondente ao EntityType
 * (ou apenas A_BOLD/A_NORMAL se o terminal não suportar cores).
 * Chamada por: drawEntity (src/draw.c).
 */
static attr_t entityAttr(EntityType type)
{
    if (!has_colors())
        return (type == ENTITY_PLAYER || type == ENTITY_BOSS) ? A_BOLD : A_NORMAL;

    switch (type)
    {
    case ENTITY_PLAYER:
        return COLOR_PAIR(COLOR_PAIR_PLAYER) | A_BOLD;
    case ENTITY_ENEMY_MELEE:
        return COLOR_PAIR(COLOR_PAIR_ENEMY);
    case ENTITY_ENEMY_RANGED:
        return COLOR_PAIR(COLOR_PAIR_ARCHER) | A_BOLD;
    case ENTITY_BOSS:
        return COLOR_PAIR(COLOR_PAIR_BOSS) | A_BOLD;
    case ENTITY_ITEM:
    case ENTITY_CONSUMABLE:
    case ENTITY_NEXT_FLOOR_DOOR:
        return COLOR_PAIR(COLOR_PAIR_ITEM) | A_BOLD;
    }
    return A_NORMAL;
}

/*
 * Desenha uma entidade ativa e na janela win aplicando a cor de entityAttr.
 * Chamada por: drawHitFlash, drawProjectilePath, drawRoomEntities e
 * drawEverything (src/draw.c).
 */
void drawEntity(Entity* e)
{
    if (!win || !e || !e->active)
        return;

    attr_t attr = entityAttr(e->type);
    if (e->type == ENTITY_ENEMY_RANGED && e->aim_turns > 0)
    {
        if (has_colors())
            attr = COLOR_PAIR(e->aim_turns >= 2 ? COLOR_PAIR_HIT : COLOR_PAIR_SHOT_ENEMY) | A_BOLD;
        else
            attr = (e->aim_turns >= 2) ? (A_REVERSE | A_BOLD) : A_BOLD;
    }
    wattron(win, attr);
    mvwaddch(win, e->pos.y, e->pos.x, e->ch);
    wattroff(win, attr);
}

/*
 * Redesenha a sala e exibe um flash vermelho 'X' na posição pos por 100 ms
 * como feedback visual ao aplicar dano.
 * Chamada por: drawProjectilePath (src/draw.c) e dealDamage (src/entity.c).
 */
void drawHitFlash(Position pos)
{
    if (!win || !curr_room)
        return;
    if (pos.x < 0 || pos.x >= ROOM_WIDTH || pos.y < 0 || pos.y >= ROOM_HEIGHT)
        return;

    drawRoom();
    drawRoomEntities();
    drawEntity(player);

    attr_t attr = has_colors() ? (COLOR_PAIR(COLOR_PAIR_HIT) | A_BOLD) : (A_REVERSE | A_BOLD);
    wattron(win, attr);
    mvwaddch(win, pos.y, pos.x, 'X');
    wattroff(win, attr);
    wrefresh(win);
    napms(100);
    flushinp();
}

/*
 * Anima o trajeto de um projétil de from até to na direção (dx, dy) usando o
 * glifo direcional ('|', '-', '/', '\') e cor de player ou inimigo; ao final,
 * aciona drawHitFlash se hit_target for verdadeiro.
 * Chamada por: fireRangedAttack (src/entity.c).
 */
void drawProjectilePath(Position from, Position to, int dx, int dy, bool is_player, bool hit_target)
{
    if (!win || !curr_room || (dx == 0 && dy == 0))
        return;

    chtype glyph = '*';
    if (dx == 0)
        glyph = '|';
    else if (dy == 0)
        glyph = '-';
    else if (dx * dy > 0)
        glyph = '\\';
    else
        glyph = '/';

    attr_t attr = A_BOLD;
    if (has_colors())
        attr |= COLOR_PAIR(is_player ? COLOR_PAIR_SHOT_PLAYER : COLOR_PAIR_SHOT_ENEMY);

    drawRoom();
    drawRoomEntities();
    drawEntity(player);
    wrefresh(win);
    flushinp();

    int x = from.x;
    int y = from.y;
    while (x != to.x || y != to.y)
    {
        x += dx;
        y += dy;
        if (x < 0 || x >= ROOM_WIDTH || y < 0 || y >= ROOM_HEIGHT)
            break;

        wattron(win, attr);
        mvwaddch(win, y, x, glyph);
        wattroff(win, attr);
        wrefresh(win);
        napms(30);

        drawRoom();
        drawRoomEntities();
        drawEntity(player);
    }

    if (hit_target)
        drawHitFlash(to);
    else
        wrefresh(win);
    flushinp();
}

/*
 * Anima o trajeto de um projétil de from até to seguindo o algoritmo de Bresenham,
 * ajustando o glifo ('|', '-', '/', '\') conforme o passo de cada iteração.
 * Chamada por: actRangedEnemy (src/entity.c).
 */
void drawBresenhamProjectilePath(Position from, Position to, bool is_player, bool hit_target)
{
    if (!win || !curr_room || (from.x == to.x && from.y == to.y))
        return;

    attr_t attr = A_BOLD;
    if (has_colors())
        attr |= COLOR_PAIR(is_player ? COLOR_PAIR_SHOT_PLAYER : COLOR_PAIR_SHOT_ENEMY);

    drawRoom();
    drawRoomEntities();
    drawEntity(player);
    wrefresh(win);
    flushinp();

    int x   = from.x;
    int y   = from.y;
    int sx  = (from.x < to.x) ? 1 : -1;
    int sy  = (from.y < to.y) ? 1 : -1;
    int adx = abs(to.x - from.x);
    int ady = abs(to.y - from.y);
    int err = adx - ady;

    while (x != to.x || y != to.y)
    {
        int nx = x;
        int ny = y;
        int e2 = 2 * err;
        if (e2 > -ady)
        {
            err -= ady;
            nx += sx;
        }
        if (e2 < adx)
        {
            err += adx;
            ny += sy;
        }
        if (nx < 0 || nx >= ROOM_WIDTH || ny < 0 || ny >= ROOM_HEIGHT)
            break;

        int    step_x = nx - x;
        int    step_y = ny - y;
        chtype glyph  = '*';
        if (step_x == 0)
            glyph = '|';
        else if (step_y == 0)
            glyph = '-';
        else if (step_x * step_y > 0)
            glyph = '\\';
        else
            glyph = '/';

        x = nx;
        y = ny;

        wattron(win, attr);
        mvwaddch(win, y, x, glyph);
        wattroff(win, attr);
        wrefresh(win);
        napms(30);

        drawRoom();
        drawRoomEntities();
        drawEntity(player);
    }

    if (hit_target)
        drawHitFlash(to);
    else
        wrefresh(win);
    flushinp();
}

/*
 * Percorre todas as entidades da sala atual e desenha cada uma via drawEntity.
 * Chamada por: drawHitFlash, drawProjectilePath e drawEverything (src/draw.c).
 */
void drawRoomEntities(void)
{
    if (!win || !map)
        return;

    MapCell* cell = &map[map_y][map_x];
    for (int i = 0; i < cell->entity_count; ++i)
    {
        drawEntity(&cell->entities[i]);
    }
}

/*
 * Atualiza a janela win_hud exibindo a barra de vida proporcional (8 blocos) e os
 * valores numéricos de HP, ataque e defesa do jogador.
 * Chamada por: drawEverything (src/draw.c).
 */
void drawHud(void)
{
    if (!win_hud || !player)
        return;

    int filled = (player->max_hp > 0) ? (player->hp * 8) / player->max_hp : 0;
    if (player->hp > 0 && filled == 0)
        filled = 1;
    if (filled < 0)
        filled = 0;
    if (filled > 8)
        filled = 8;

    char bar[9] = "--------";
    for (int i = 0; i < filled; ++i)
        bar[i] = '#';

    char hp_text[16];
    int  len  = snprintf(hp_text, sizeof(hp_text), "%d / %d", player->hp, player->max_hp);
    int  hp_x = 11 + (10 - len) / 2;
    if (hp_x < 1)
        hp_x = 1;

    werase(win_hud);
    box(win_hud, 0, 0);
    mvwprintw(win_hud, 0, 3, " STATUS DO JOGADOR ");
    mvwprintw(win_hud, 2, 3, "♥ VIDA: [%s]", bar);
    mvwprintw(win_hud, 3, hp_x, "%s", hp_text);
    mvwprintw(win_hud, 6, 3, "⚔ ATAQUE: %d", player->attack);
    mvwprintw(win_hud, 8, 3, "⛨ DEFESA: %d", player->defense);
}

/*
 * Consome e imprime sequencialmente as mensagens da fila logs via logDefault,
 * pausando com getch() caso ainda existam mais mensagens pendentes.
 * Chamada por: drawEverything (src/draw.c).
 */
void drawLogs()
{
    if (!logs)
        return;

    while (logs->size > 0)
    {
        char* msg = popLogQueue(logs);
        if (msg)
            logDefault(msg);
        // se nao for o ultimo
        refresh();
        if (logs->size > 0)
        {
            curs_set(1);
            getch();
            curs_set(0);
        }
    }
}

/*
 * Verifica o tamanho mínimo do terminal (77x14) e redesenha em lote a sala,
 * entidades, jogador, minimapa, HUD e logs usando wnoutrefresh/doupdate.
 * Chamada por: gameLoop (src/engine.c).
 */
void drawEverything(void)
{
    werase(stdscr);
    wnoutrefresh(stdscr);

    if (COLS < 77 || LINES < ROOM_HEIGHT + 3 || !win || !win_minimapa || !win_hud)
    {
        if (LINES > 0 && COLS > 0)
        {
            const char* msg = "Terminal pequeno! Min: 77x14";
            int         y   = LINES / 2;
            int         x   = (COLS - (int)strlen(msg)) / 2;
            if (x < 0)
                x = 0;
            mvaddnstr(y, x, msg, COLS - x);
            wnoutrefresh(stdscr);
        }
        doupdate();
        return;
    }

    drawRoom();
    drawRoomEntities();
    drawEntity(player);
    drawMinimap();
    drawHud();

    touchwin(win);
    touchwin(win_minimapa);
    touchwin(win_hud);
    wnoutrefresh(win);
    wnoutrefresh(win_minimapa);
    wnoutrefresh(win_hud);
    doupdate();
    drawLogs();
}
