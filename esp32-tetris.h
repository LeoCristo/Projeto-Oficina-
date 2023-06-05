/*
  Definições de tamanho do painel, linhas, colunas, número de peças, velocidade de movimento de peças, etc
*/
// A_0 le o horizontal do joystick e A_1 o vertical
#define joystick_x          (34)// pino leitor do joystick em x
#define joystick_y          (35)// pino leitor do joystick em y
#define GRID_W              (8) //comprimento da linha
#define GRID_H              (16)//quantidade de linhas
#define STRAND_LENGTH       (GRID_W*GRID_H) //tamanho do jogo 128 leds
#define LED_DATA_PIN        (15)// GPIO 15 conectado na entrada de dados do painel de leds WS2812
#define BACKWARDS           (0)//Se o painel for colocado  em S ao invés de Z mudar o valor para 0
#define PIECE_W             (4)//tamhno máximo de cada peça do tetris
#define PIECE_H             (4)
#define NUM_PIECE_TYPES     (7)//Número de peças
#define JOYSTICK_DEAD_ZONE  (30)//Quantidade que o joystick tem que variar para começar a ser interpretado
#define JOYSTICK_PIN        (2)//botao do joystick
#define DROP_MINIMUM        (25)  // velocidade maxima de caida
#define DROP_ACCELERATION   (20)  // taxa de aumento da caida de peças
#define INITIAL_MOVE_DELAY  (1000) 
#define INITIAL_DROP_DELAY  (1500)
#define INITIAL_DRAW_DELAY  (30)
//Entradas e saídas

#define button_pause        (18)        // Botão de pause
#define button_start        (19)        // Botão de start

