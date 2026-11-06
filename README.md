# Trabalho Final - Laboratório de Programação

## Guia de sobrevivência:

### Diferença entre arquivo .c e .h:

- .h:
  Definem a existência de uma função, seu "protótipo". Não têm memória alocada e nem definem o comportamento da função, apenas declaram sua existência. O arquivo **.h** promete a existência de uma função. Vai no cabeçalho dos arquivos. Seu uso pode ser visto nas bibliotecas, ferramentas essenciais para codar.

  Por exemplo, em determinado arquivo teste.h, existe:
  
  ´´´
  int somar(int a, int b);
  ´´´
  
  O que isso faz é literalmente dizer que "existe uma função chamada 'somar' que recebe dois inteiros a e b e devolve um inteiro".

- .c:
  Implementa as funcionalidades que foram "prometidas" em um .h. Aqui está toda lógica, loops, operações... etc.

´´´
#include "calculadora.h"

int somar(int a, int b) {
    return a + b;
}
´´´

O arquivo .h permite maior organização e modularização, pois ao incluir um .h, torna-se sabido o que a função precisa e o seu retorno sem precisar ter seu funcionamento definido. Depois disso o compilador simplesmente faz alguma coisa aí escrever direito depois.  

### 1. por onde começar: rogue.h
  Se for pra começar por algum canto, tem que começar entendendo as variáveis fundamentais (e principais).
    Tudo se baseia em:
      1. Position
      2. Entity
      3. Tile
      4. MapCell
  O resto, é buildado em volta do ncurses, que é meio que o nosso "motor gráfico". Ele não dá nada de mal beijada pra gente, é uma ferramenta que abstrai o terminal. Ele é utilizado para termos estas interações com o terminal de como se estivessimos em uma UI baseada em teclado.
  
  Dito isso, voltando para as variáveis fundamentais:
    Position são literalmente coordenadas.
     
    Toda entidade tem:
    1. Uma position no mapa
    2. Um caracter de representação
     
    Cada ponto em que uma entidade pisa (Tile), possui um caracter de representação e um valor lógico para checar se é possível andar por ali ou não.
    
    Uma MapCell representa uma sala.
    Toda sala é uma matriz de caracteres demilitado porr uma constate de largura e altura.
    Toda sala possui:
    1. ID.
    2. Uma matriz de Tiles onde cada sala possui valores lógicos para visitado ou descoberto.
### 2. Main.c
  Considerações iniciais:
    1. Globalmente temos:
      - Um player.
      - Uma matriz de Tiles, os quadradinhos. Basicamente uma sala.
      - Duas janelas (isso só mais pra frente)
      - Uma matriz que contem as salas.

    2. Dentro da MAIN:
    **Criação das janelas do ncurses, alocamento de memória para o andar, preenchimento do andar com salas, criação e posicionamento do player**
        - cursesSetup(): inicia a visualização "em modo de UI" (não sei nome melhor para dar a isso).
        - Cria variáveis que mais para frente estaram sendo utilizadas como as proporções e coordenadas das "Janelas".
        - Criamos o minimapa e a janela de visualisação do player com as variáveis previamente definidas.
        - A função createMap() aloca uma matriz, que basicamente servirá para receber as salas futuras.
        - SetupFloor preenche o mapa (a matriz que previamente haviamos alocada e que chamaremos de andar) com salas. Salas estas que seguem o padrão predefinido (por enquanto).
        - curr_room define como sendo a sala atual, à sala map[map_y][map_x].tiles do nosso mapa previamente criado.
        - Cria um player, definindo sua posição inicial.
    **Antes de fechar a main, looping, refresh e quit**
        -
