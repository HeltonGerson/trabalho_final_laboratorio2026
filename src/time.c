#include "rogue.h"

/*
 * TEMPO:
 * 1: Ver quais acoes passam o tempo, Acoes do player validas
 * 2: Fazer um vetor / fila com todas as entidades.
 * 3: Cada entidade vai ter sua acao
 * 3.1: Os inimigos vao perseguir o player
 * 3.2(depois): Os projeteis seguem o seu percurso
 */

/*
 * Executa o turno de todas as entidades ativas na sala atual enquanto o jogador
 * estiver vivo, despachando a IA de cada tipo (chasePlayer, actRangedEnemy, actBoss).
 * Chamada por: handleInput e movePlayer (src/entity.c), e tests/test_floor.c.
 */
void passTime(void)
{
    if (!map || map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT)
        return;
    if (!player || !player->active || player->hp <= 0)
        return;

    MapCell* cell      = &map[map_y][map_x];
    Entity*  entities  = cell->entities;
    int      ent_count = cell->entity_count;
    for (int i = 0; i < ent_count; i++)
    {
        if (!player->active || player->hp <= 0)
            break;

        Entity* e = &entities[i];
        if (!e->active)
            continue;

        switch (e->type)
        {
        case ENTITY_ENEMY_MELEE:
            chasePlayer(e);
            break;
        case ENTITY_ENEMY_RANGED:
            actRangedEnemy(e);
            break;
        case ENTITY_BOSS:
            actBoss(e);
            break;
        default:
            break;
        }
    }
}
