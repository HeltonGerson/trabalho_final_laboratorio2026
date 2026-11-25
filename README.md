# Dungeon C — Exploração de Masmorras

Projeto final da disciplina de **Laboratório de Programação** (Ciência da Computação — 2026.2).  
**Equipe:** Helton, Lucas Gabriel e Francisco Rerisson.

---

## Visão Geral

**Dungeon C** é um roguelike por turnos em terminal Linux escrito em **C99 puro** com interface em **`ncurses`**. O jogador explora andares gerados proceduralmente, enfrenta inimigos com comportamentos distintos e coleta itens para fortalecer seus atributos.

---

## Guia Rápido: Como o Jogo Funciona

### 1. Fluxo da Partida
1. **Menu Inicial (`showMainMenu`):** permite iniciar um novo jogo (`Start New Game`), continuar (`Continue`, quando existir save em `data/save/save.txt`) ou sair (`Quit`).
2. **Geração do Andar (`setupFloor`):** cria uma grade `7x7` e gera de **8 a 15 salas** conectadas usando busca em profundidade (DFS) com pilha (`coordStack`), garantindo a sala inicial, ao menos uma sala de itens e uma sala de Boss.
3. **Loop de Turnos (`gameLoop`):** cada movimento ou disparo do jogador executa um turno das entidades vivas na sala (`passTime`), redesenha as janelas e exibe os logs de combate. Enquanto houver inimigos vivos na sala (`roomHasAliveEnemies`), as portas permanecem trancadas. Ao sair vivo pressionando `ESC`, o estado da partida é salvo automaticamente (`save` / `continueGame` em `data/save/save.txt`).
4. **Progressão de Andares (`newFloor`):** o jogo possui **2 andares** com escalonamento de atributos dos inimigos a partir do 2º andar. Ao derrotar o Boss, surge a porta para o próximo andar (`ENTITY_NEXT_FLOOR_DOOR`).
5. **Vitória e Game Over (`winGame` / `showGameOver`):** ao derrotar o Boss do último andar e confirmar a saída pela porta final, a tela de vitória exibe os atributos finais do jogador; ao zerar o HP, a tela de Game Over permite voltar ao menu ou sair.

### 2. Interface no Terminal (mínimo `77x14`)
- **Minimapa (esquerda):** janela deslizante `3x3` com névoa de guerra (`[@]` atual, `[S]` início, `[B]` boss, `[I]` itens, `[X]` normal visitada, `[?]` adjacente descoberta, `###` borda do mapa).
- **Sala (centro):** área `31x11` em escala `1:1` carregada dos templates ASCII em `data/rooms/`, com portas nas bordas conectadas às salas vizinhas.
- **HUD (direita):** barra de vida (`HP / Max HP`), `ATAQUE` e `DEFESA` atuais do jogador.
- **Logs (rodapé):** mensagens de combate e eventos exibidas na penúltima linha do terminal.

### 3. Entidades, Combate e IA
- **`@` Jogador:** inicia com `20 HP`, `4 ATK` (corpo a corpo), `2 RANGED` (arco com alcance `8`, sem cooldown) e `0 DEF`.
  - **Movimento e corpo a corpo:** move-se em 8 direções (`WASD` nas cardeais e `Q`/`E`/`Z`/`C` nas diagonais); mover-se sobre um inimigo desfere um ataque corpo a corpo.
  - **À distância:** as setas direcionais disparam uma flecha nas 4 direções cardeais em linha reta (para em paredes `#` ou inimigos).
  - **Prioridade de ataque:** ao entrar na linha de ataque de um inimigo nesse turno, o jogador tem prioridade e não sofre o golpe imediato.
- **`E` Goblin Melee (`6 HP`, `4 ATK`, `0 DEF`):** persegue o jogador pelo menor caminho usando busca em largura (BFS) em 8 direções, ataca quando adjacente (cardeais e diagonais) e tem chance de dropar um consumível de cura (`C`) ao morrer.
- **`A` Goblin Arqueiro (`6 HP`, `3 ATK`, `2 RANGED`, `0 DEF`):** permanece parado, possui visão 360° via algoritmo de Bresenham (alcance `5`), mira por 2 turnos avisando no log, dispara no 3º turno e recarrega por `3` turnos; também tem chance de dropar um consumível de cura (`C`) ao morrer.
- **`B` Boss (`35 HP`, `7 ATK`, `5 RANGED`, `1 DEF`, alcance `8`):** ataca corpo a corpo quando adjacente, dispara em 8 direções quando há linha de visão (com cooldown de `1` turno e aviso prévio se o jogador acabou de entrar na mira) e persegue via BFS caso contrário. Ao morrer, libera a passagem para o próximo andar ou para a vitória.
- **Escalonamento entre andares:** a partir do 2º andar, inimigos e Boss recebem bônus progressivo de HP, ataque e defesa.
- **`I` Itens e `C` Consumíveis:** caminhar sobre `I` concede bônus permanentes (HP máximo, ataque corpo a corpo, ataque do arco ou defesa); caminhar sobre `C` recupera HP.

---

## Controles

| Ação | Teclas |
| :--- | :--- |
| **Mover / Atacar corpo a corpo (8 direções)** | `W`, `A`, `S`, `D` (cardeais) e `Q`, `E`, `Z`, `C` (diagonais) |
| **Disparar arco (4 direções)** | Setas direcionais (`↑`, `↓`, `←`, `→`) |
| **Navegar nos menus** | `W`/`S`, `k`/`j` ou `↑`/`↓` + `Enter` |
| **Confirmar avanço de andar / vitória** | `y` |
| **Sair da partida (salvando) / menu** | `ESC` (`q` nas telas de Game Over e Vitória) |

---

## Estrutura do Projeto

```text
.
├── makefile             # Compilação, execução e testes (-Wall -Wextra -pedantic -std=c99)
├── include/rogue.h      # Structs, enums, constantes, globais e protótipos
├── data/rooms/          # Templates ASCII (start/, boss/, items/, normal/)
├── src/
│   ├── main.c           # Ponto de entrada e gerenciamento das janelas ncurses
│   ├── engine.c         # Setup do ncurses, menus, gameLoop, save/load (data/save/save.txt), vitória e limpeza de memória
│   ├── draw.c           # Renderização da sala, minimapa, HUD, projéteis e logs
│   ├── map.c            # Geração procedural do andar (DFS) e névoa de guerra
│   ├── room.c           # Leitura de templates ASCII e controle de portas
│   ├── entity.c         # Entidades, itens, movimento, combate e IA (BFS/ranged/boss)
│   ├── time.c           # Loop de turno dos inimigos (passTime)
│   ├── log.c            # Exibição de mensagens no rodapé
│   └── structs.c        # Pilha (coordStack) e filas (coordQueue, logQueue)
└── tests/
    └── test_floor.c     # Suíte com 16 testes automatizados (mapa procedural, entidades, combate, IA, save/load, vitória e tranca de portas)
```

---

## Compilação, Execução e Testes

**Dependências:** compilador C99 (`gcc`) e biblioteca `ncurses` com suporte a caracteres largos (`libncursesw5-dev` no Debian/Ubuntu, `ncurses-devel` no Fedora ou `ncurses` no Arch Linux).

```bash
make compile   # Compila o binário ./game sem warnings
make run       # Executa ./game
make           # Compila, executa e limpa os binários
make test      # Compila e executa os 16 testes automatizados
make clean     # Remove binários gerados (game e test_runner)
```
