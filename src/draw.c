#include "rogue.h"

/*
 * Escolhe qual letra vai representar a sala no minimapa ('S' para início, 'B'
 * para a sala do chefão, 'I' para sala de item ou 'X' para sala comum).
 * Onde é chamada: drawMinimap (src/draw.c).
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
 * Desenha todos os quadradinhos da sala atual (chão, paredes e portas) na
 * janela da sala.
 * Onde é chamada: drawHitFlash, drawProjectilePath, drawBresenhamProjectilePath
 * e drawEverything (src/draw.c).
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
 * Desenha o minimapa mostrando uma grade 3x3 em volta da sala onde o jogador
 * está, marcando o jogador ([@]), as salas já visitadas, as salas vizinhas
 * descobertas ([?]), os limites fora do mapa (###) e os caminhos ('-' e '|').
 * Onde é chamada: drawEverything (src/draw.c).
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
 * Escolhe a cor e o destaque na tela para cada tipo de personagem ou item (ou
 * usa apenas negrito/normal se o terminal não tiver suporte a cores).
 * Onde é chamada: drawEntity (src/draw.c).
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
 * Desenha um personagem ou item ativo na janela da sala usando a cor certa
 * (mudando a cor do arqueiro quando ele está mirando para avisar o jogador).
 * Onde é chamada: drawHitFlash, drawProjectilePath, drawBresenhamProjectilePath,
 * drawRoomEntities e drawEverything (src/draw.c).
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
 * Redesenha a sala e faz piscar um 'X' vermelho por um instante no lugar onde
 * alguém levou dano.
 * Onde é chamada: drawProjectilePath e drawBresenhamProjectilePath (src/draw.c),
 * e dealDamage (src/entity.c).
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
 * Mostra a animação de uma flecha ou tiro indo em linha reta de um ponto ao
 * outro, usando um tracinho virado para a direção do tiro ('|', '-', '/', '\')
 * na cor do jogador ou do inimigo; se acertar, faz piscar o 'X' de dano.
 * Onde é chamada: fireRangedAttack (src/entity.c).
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
 * Mostra a animação de um tiro indo em linha reta de um ponto ao outro em
 * qualquer ângulo, ajustando o desenho do tracinho ('|', '-', '/', '\') a cada
 * passo até o destino, e faz piscar o 'X' de dano se acertar.
 * Onde é chamada: actRangedEnemy (src/entity.c).
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
 * Passa por todos os inimigos, itens e portas especiais da sala atual e desenha
 * cada um na tela.
 * Onde é chamada: drawHitFlash, drawProjectilePath, drawBresenhamProjectilePath
 * e drawEverything (src/draw.c).
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
 * Atualiza o painel lateral de status com a barrinha de vida (8 bloquinhos) e os
 * números de vida, ataque e defesa do jogador.
 * Onde é chamada: drawEverything (src/draw.c).
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
    mvwprintw(win_hud, 7, 3, "→ ARCO: %d", player->ranged_attack);
    mvwprintw(win_hud, 8, 3, "⛨ DEFESA: %d", player->defense);
}

/*
 * Mostra na tela, uma por uma, as mensagens que estão esperando na fila; se
 * ainda houver mais mensagens depois da atual, espera o jogador apertar uma
 * tecla para continuar.
 * Onde é chamada: drawEverything (src/draw.c).
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
        // Se ainda tiver mais mensagens depois desta, espera uma tecla antes de mostrar a próxima
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
 * Confere se a janela do terminal tem espaço suficiente (pelo menos 77 por 14)
 * e redesenha tudo de uma vez na tela: sala, personagens, jogador, minimapa,
 * painel de status e mensagens.
 * Onde é chamada: gameLoop (src/engine.c).
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
