#include "rogue.h"

/*
 * Controla a passagem de tempo do jogo: a cada ação do jogador, todos os
 * inimigos da sala ganham a vez de agir.
 */

/*
 * Faz passar um turno na sala atual: enquanto o jogador estiver vivo, dá a vez
 * para cada inimigo ativo agir (o goblin de perto persegue, o arqueiro mira e
 * atira e o chefão age).
 * Onde é chamada: handleInput e movePlayer (src/entity.c), e tests/test_floor.c.
 */
void passTime(void)
{
    if (!map || map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT)
        return;
    if (!player || !player->active || player->hp <= 0)
        return;

    MapCell* cell = &map[map_y][map_x];
    for (int i = 0; i < cell->entity_count; i++)
    {
        if (!player->active || player->hp <= 0)
            break;

        Entity* e = &cell->entities[i];
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
            actBossI(e);
            break;
        default:
            break;
        }
    }
}
