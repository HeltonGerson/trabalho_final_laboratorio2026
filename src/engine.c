#include "rogue.h"
#include <curses.h>
#include <locale.h>
#include <stdio.h>

/*
 * Configura o locale UTF-8, inicializa o modo ncurses (sem eco, cbreak, keypad,
 * cursor oculto) e registra os pares de cores usados em entidades, tiros e dano.
 * Chamada por: main (src/main.c).
 */
void cursesSetup(void)
{
    setlocale(LC_ALL, "");
    initscr();            /* Inicia o modo curses */
    noecho();             /* Faz com que eh digitado nao apareca no term */
    cbreak();             /* Comandos nao sao executados (ex: Ctrl-C) */
    keypad(stdscr, TRUE); /* Permite pegar caracteres como as setas e teclas F(n) */
    curs_set(0);          /* Cursor nao aparece */
    if (has_colors())
    {
        start_color();
        use_default_colors();
        init_pair(COLOR_PAIR_PLAYER, COLOR_GREEN, -1);
        init_pair(COLOR_PAIR_ENEMY, COLOR_RED, -1);
        init_pair(COLOR_PAIR_ARCHER, COLOR_YELLOW, -1);
        init_pair(COLOR_PAIR_BOSS, COLOR_MAGENTA, -1);
        init_pair(COLOR_PAIR_ITEM, COLOR_CYAN, -1);
        init_pair(COLOR_PAIR_SHOT_PLAYER, COLOR_CYAN, -1);
        init_pair(COLOR_PAIR_SHOT_ENEMY, COLOR_RED, -1);
        init_pair(COLOR_PAIR_HIT, COLOR_WHITE, COLOR_RED);
    }
}

/*
 * Verifica se o arquivo de save (SAVE_PATH) existe e pode ser aberto para leitura.
 * Chamada por: showMainMenu e continueGame (src/engine.c).
 */
bool saveExists(void)
{
    FILE* f = fopen(SAVE_PATH, "r");
    if (!f)
        return false;
    fclose(f);
    return true;
}

/*
 * Salva o estado completo do andar (salas, entidades e seus atributos de combate
 * e estado de visitação) e do jogador (atributos e posição no mapa) em SAVE_PATH.
 * Chamada por: main (src/main.c) e tests/test_floor.c.
 */

void eraseSave()
{
    if (!saveExists())
        remove(SAVE_PATH);
}

void save(Entity* p)
{
    if (!p || !map || !p->active || p->hp <= 0)
        return;

    FILE* fp = fopen(SAVE_PATH, "w");
    if (!fp)
        return;

    int somaEntidades = 0;
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            somaEntidades += map[i][j].entity_count;
        }
    }
    fprintf(fp, "%d\n", somaEntidades);

    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            fprintf(fp, "%d %d %d\n", map[i][j].id, map[i][j].entity_count, (int)map[i][j].visited);
            for (int t = 0; t < map[i][j].entity_count; t++)
            {
                Entity* e = &map[i][j].entities[t];
                fprintf(fp, "%d %c %d %d %d %d %d %d %d %d %d %d %d\n", (int)e->active, (char)e->ch,
                        (int)e->type, e->attack, e->ranged_attack, e->ranged_range,
                        e->ranged_cooldown, e->defense, e->hp, e->max_hp, e->healingItem, e->pos.x,
                        e->pos.y);
            }
        }
    }

    fprintf(fp, "%d %d %d %d %d %d %d %d %d %d %d\n", p->hp, p->max_hp, p->attack, p->ranged_attack,
            p->ranged_range, p->ranged_cooldown, p->defense, p->pos.x, p->pos.y, map_y, map_x);

    fclose(fp);
}

/*
 * Carrega o estado salvo de SAVE_PATH para map e player, reconstrói os tiles
 * de cada sala via readRoom(), limpa marcadores de entidades e atualiza o fog of war.
 * Chamada por: main (src/main.c) e tests/test_floor.c.
 */
bool continueGame(void)
{
    if (!saveExists() || !map)
        return false;

    FILE* fp = fopen(SAVE_PATH, "r");
    if (!fp)
        return false;

    int totalEntities = 0;
    if (fscanf(fp, "%d", &totalEntities) != 1 || totalEntities < 0)
    {
        fclose(fp);
        return false;
    }

    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            int countRoom   = 0;
            int countEntity = 0;
            int auxVisited  = 0;
            if (fscanf(fp, "%d %d %d", &countRoom, &countEntity, &auxVisited) != 3 ||
                countEntity < 0)
            {
                fclose(fp);
                return false;
            }

            map[i][j].id           = countRoom;
            map[i][j].entity_count = 0;
            map[i][j].visited      = (auxVisited != 0);
            map[i][j].discovered   = false;

            if (countRoom != 0)
            {
                map[i][j].tiles = readRoom(countRoom);
                if (!map[i][j].tiles)
                {
                    fclose(fp);
                    return false;
                }
                removeEntityChars(&map[i][j]);

                if (countEntity > 0)
                {
                    map[i][j].entities = malloc((size_t)countEntity * sizeof(Entity));
                    if (!map[i][j].entities)
                    {
                        fclose(fp);
                        return false;
                    }
                    map[i][j].entity_count = countEntity;

                    for (int t = 0; t < countEntity; t++)
                    {
                        Entity auxEntity;
                        int    auxB = 0;
                        int    auxT = 0;
                        char   auxC = '\0';

                        if (fscanf(fp, "%d %c %d %d %d %d %d %d %d %d %d %d %d", &auxB, &auxC,
                                   &auxT, &auxEntity.attack, &auxEntity.ranged_attack,
                                   &auxEntity.ranged_range, &auxEntity.ranged_cooldown,
                                   &auxEntity.defense, &auxEntity.hp, &auxEntity.max_hp,
                                   &auxEntity.healingItem, &auxEntity.pos.x,
                                   &auxEntity.pos.y) != 13)
                        {
                            fclose(fp);
                            return false;
                        }

                        auxEntity.active      = (auxB != 0);
                        auxEntity.ch          = (chtype)auxC;
                        auxEntity.type        = (EntityType)auxT;
                        auxEntity.aim_turns   = 0;
                        map[i][j].entities[t] = auxEntity;
                    }
                }
            }
        }
    }

    player = malloc(sizeof(Entity));
    if (!player)
    {
        fclose(fp);
        return false;
    }

    player->healingItem = 0;
    player->aim_turns   = 0;
    player->active      = true;
    player->ch          = '@';
    player->type        = ENTITY_PLAYER;

    if (fscanf(fp, "%d %d %d %d %d %d %d %d %d %d %d", &player->hp, &player->max_hp,
               &player->attack, &player->ranged_attack, &player->ranged_range,
               &player->ranged_cooldown, &player->defense, &player->pos.x, &player->pos.y, &map_y,
               &map_x) != 11)
    {
        fclose(fp);
        return false;
    }

    fclose(fp);

    if (map_y < 0 || map_y >= MAP_HEIGHT || map_x < 0 || map_x >= MAP_WIDTH ||
        map[map_y][map_x].id == 0 || !map[map_y][map_x].tiles)
    {
        return false;
    }

    updateFogOfWar();
    return true;
}

/*
 * Exibe o menu principal centralizado (Start New Game, Continue se saveExists()
 * for verdadeiro, e Quit), trata a navegação por teclado e retorna a MenuOption.
 * Chamada por: main (src/main.c).
 */
MenuOption showMainMenu(void)
{
    const char* labels[3];
    MenuOption  opts[3];
    int         count    = 0;
    int         selected = 0;

    labels[count] = "Start New Game";
    opts[count++] = MENU_NEW_GAME;
    if (saveExists())
    {
        labels[count] = "Continue";
        opts[count++] = MENU_CONTINUE;
    }
    labels[count] = "Quit";
    opts[count++] = MENU_QUIT;
    while (1)
    {
        werase(stdscr);
        int start_y = (LINES - (count + 3)) / 2;
        if (start_y < 0)
            start_y = 0;

        const char* title = "===  DUNGEON C ===";
        int         tx    = (COLS - (int)strlen(title)) / 2;
        if (tx < 0)
            tx = 0;
        mvprintw(start_y, tx, "%s", title);

        for (int i = 0; i < count; ++i)
        {
            int y = start_y + 2 + i;
            if (y >= LINES)
                break;
            int x = (COLS - (int)strlen(labels[i]) - 5) / 2;
            if (x < 0)
                x = 0;
            if (i == selected)
                attron(A_REVERSE);
            mvprintw(y, x, " %s %s  ", (i == selected) ? ">" : " ", labels[i]);
            if (i == selected)
                attroff(A_REVERSE);
        }
        refresh();

        int ch = getch();
        if (ch == KEY_UP || ch == 'w' || ch == 'k')
            selected = (selected - 1 + count) % count;
        else if (ch == KEY_DOWN || ch == 's' || ch == 'j')
            selected = (selected + 1) % count;
        else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER)
        {
            return opts[selected];
        }
        else if ((int)ch == 27)
        {
            return MENU_QUIT;
        }
    }
}

/*
 * Limpa o buffer de entrada (flushinp), exibe a tela de Game Over com as opções
 * "Voltar ao Menu" e "Sair" e retorna true para reiniciar ou false para sair.
 * Chamada por: main (src/main.c).
 */
bool showGameOver(void)
{
    const char* labels[2] = {"Voltar ao Menu", "Sair"};
    int         count     = 2;
    int         selected  = 0;

    flushinp();

    while (1)
    {
        werase(stdscr);
        int start_y = (LINES - (count + 3)) / 2;
        if (start_y < 0)
            start_y = 0;

        const char* title = "=== GAME OVER ===";
        int         tx    = (COLS - (int)strlen(title)) / 2;
        if (tx < 0)
            tx = 0;
        mvprintw(start_y, tx, "%s", title);

        for (int i = 0; i < count; ++i)
        {
            int y = start_y + 2 + i;
            if (y >= LINES)
                break;
            int x = (COLS - (int)strlen(labels[i]) - 5) / 2;
            if (x < 0)
                x = 0;
            if (i == selected)
                attron(A_REVERSE);
            mvprintw(y, x, " %s %s  ", (i == selected) ? ">" : " ", labels[i]);
            if (i == selected)
                attroff(A_REVERSE);
        }
        refresh();

        int ch = getch();
        if (ch == KEY_UP || ch == 'w' || ch == 'k')
            selected = (selected - 1 + count) % count;
        else if (ch == KEY_DOWN || ch == 's' || ch == 'j')
            selected = (selected + 1) % count;
        else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER)
            return selected == 0;
        else if (ch == 'q')
            return false;
    }
}

bool winGame(void)
{
    const char* labels[2] = {"Back to Menu", "Quit"};
    int         selected  = 0;
    int         hp        = player ? player->hp : 0;
    int         max_hp    = player ? player->max_hp : 0;
    int         atk       = player ? player->attack : 0;
    int         def       = player ? player->defense : 0;
    char        stats[48];

    eraseSave();
    flushinp();
    snprintf(stats, sizeof(stats), "HP: %d/%d  |  ATK: %d  |  DEF: %d", hp, max_hp, atk, def);

    while (1)
    {
        werase(stdscr);
        refresh();

        int     ch = 0;
        WINDOW* win_vic =
            (LINES >= 13 && COLS >= 46) ? newwin(13, 46, (LINES - 13) / 2, (COLS - 46) / 2) : NULL;

        if (win_vic)
        {
            const char* title  = "=== VICTORY! DUNGEON C ===";
            const char* sub1   = "The Goblin Specialist was defeated!";
            const char* sub2   = "You conquered the dungeon.";
            const char* hint   = "[W/S] Move   |  [Enter] Confirm";
            int         menu_x = (46 - (int)strlen(labels[0]) - 4) / 2;

            keypad(win_vic, TRUE);
            wborder(win_vic, '|', '|', '-', '-', '+', '+', '+', '+');

            if (has_colors())
                wattron(win_vic, COLOR_PAIR(COLOR_PAIR_ARCHER) | A_BOLD);
            mvwprintw(win_vic, 1, (46 - (int)strlen(title)) / 2, "%s", title);
            if (has_colors())
                wattroff(win_vic, COLOR_PAIR(COLOR_PAIR_ARCHER) | A_BOLD);

            mvwprintw(win_vic, 3, (46 - (int)strlen(sub1)) / 2, "%s", sub1);
            mvwprintw(win_vic, 4, (46 - (int)strlen(sub2)) / 2, "%s", sub2);

            if (has_colors())
                wattron(win_vic, COLOR_PAIR(COLOR_PAIR_PLAYER));
            mvwprintw(win_vic, 6, (46 - (int)strlen(stats)) / 2, "%s", stats);
            if (has_colors())
                wattroff(win_vic, COLOR_PAIR(COLOR_PAIR_PLAYER));

            for (int i = 0; i < 2; ++i)
            {
                if (i == selected)
                    wattron(win_vic, A_REVERSE);
                mvwprintw(win_vic, 8 + i, menu_x, " %s %s ", (i == selected) ? ">" : " ",
                          labels[i]);
                if (i == selected)
                    wattroff(win_vic, A_REVERSE);
            }

            mvwprintw(win_vic, 11, (46 - (int)strlen(hint)) / 2, "%s", hint);
            wrefresh(win_vic);
            ch = wgetch(win_vic);
            delwin(win_vic);
        }
        else
        {
            const char* title   = "=== VICTORY! DUNGEON C ===";
            int         start_y = (LINES - 5) / 2;
            int         menu_x  = (COLS - (int)strlen(labels[0]) - 5) / 2;
            if (start_y < 0)
                start_y = 0;
            if (menu_x < 0)
                menu_x = 0;
            mvprintw(start_y,
                     (COLS - (int)strlen(title)) / 2 > 0 ? (COLS - (int)strlen(title)) / 2 : 0,
                     "%s", title);
            for (int i = 0; i < 2; ++i)
            {
                int y = start_y + 2 + i;
                if (y >= LINES)
                    break;
                if (i == selected)
                    attron(A_REVERSE);
                mvprintw(y, menu_x, " %s %s  ", (i == selected) ? ">" : " ", labels[i]);
                if (i == selected)
                    attroff(A_REVERSE);
            }
            refresh();
            ch = getch();
        }

        if (ch == KEY_UP || ch == 'w' || ch == 'k')
            selected = (selected - 1 + 2) % 2;
        else if (ch == KEY_DOWN || ch == 's' || ch == 'j')
            selected = (selected + 1) % 2;
        else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER)
            return selected == 0;
        else if (ch == 'q' || ch == 27)
            return false;
    }
}

/*
 * Executa o loop principal da partida enquanto o jogador estiver ativo e vivo:
 * lê comandos, trata saída ('q'), redimensionamento (KEY_RESIZE) ou ações
 * (handleInput), redesenha a tela e retorna true se o player morreu ou false se saiu.
 * Chamada por: main (src/main.c).
 */
bool gameLoop(void)
{
    int ch;

    drawEverything();

    while (player && player->active && player->hp > 0 && !game_won)
    {
        ch = getch();
        if (ch == 27)
            return false;

        if (ch == KEY_RESIZE)
            layoutWindows();
        else if (win && win_minimapa && win_hud)
            handleInput(ch);

        if (game_won)
            return false;

        drawEverything();

        if (!player || !player->active || player->hp <= 0)
            return true;
    }

    return true;
}

/*
 * Libera todos os recursos alocados para uma partida (janelas, player, fila logs,
 * matriz map e zera curr_room).
 * Chamada por: main (src/main.c) e closeGame (src/engine.c).
 */
void cleanupRun(void)
{
    destroyWindows();

    free(player);
    player = NULL;

    if (logs)
    {
        eraseLogQueue(logs);
        logs = NULL;
    }

    freeMap();
    curr_room = NULL;
}

/*
 * Termina o jogo: libera toda a memória da partida via cleanupRun e encerra o
 * modo ncurses com endwin().
 * Chamada por: main (src/main.c).
 */
void closeGame(void)
{
    cleanupRun();
    endwin();
}
