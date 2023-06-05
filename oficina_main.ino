/*------------------------------------------------------------------------------
 8x16 Tetris 
 WS2812 conectado em S
 8  leds por linha
 16 linhas
 128 leds
 esp de 30 pinos
//------------------------------------------------------------------------------*/
#include <Arduino.h>
#include "Adafruit_NeoPixel.h"  
#include "EEPROMAnything.h"   
#include "esp32-tetris.h"     
#include "DFRobotDFPlayerMini.h"
//Utilizada pelo tocador de mp3 (RX2 E TX2)
HardwareSerial mySoftwareSerial(1); /* UART1 (0), UART2 (1), UART3 (2). */  

DFRobotDFPlayerMini myDFPlayer; /* Cria a variável "myDFPlayer" para o audio*/

int score = 0;//pontuação

bool Pause = false; // Identifica jogo pausado
bool pause_onece = false;// 
bool pause_pressed = false;//Identifica se o botão de pause foi pressionado
unsigned long previousMillis = 0; //Timer milis medição anterior
unsigned long currentMillis = 0; // Timer milis medição atual ou corrente

// Cada peça tem 4 de comprimento e 4 de altura e possui 4 rotações feitas pelo joystick
// Representa a posição da peça em cada rotação = total 4
const char piece_I[] = {
  0,0,0,0,
  1,1,1,1,
  0,0,0,0,
  0,0,0,0,

  0,0,1,0,
  0,0,1,0,
  0,0,1,0,
  0,0,1,0,
  
  0,0,0,0,
  0,0,0,0,
  1,1,1,1,
  0,0,0,0,

  0,1,0,0,
  0,1,0,0,
  0,1,0,0,
  0,1,0,0,
};

const char piece_L[] = {
  0,0,1,0,
  1,1,1,0,
  0,0,0,0,
  0,0,0,0,
  
  0,1,0,0,
  0,1,0,0,
  0,1,1,0,
  0,0,0,0,

  0,0,0,0,
  1,1,1,0,
  1,0,0,0,
  0,0,0,0,
  
  1,1,0,0,
  0,1,0,0,
  0,1,0,0,
  0,0,0,0,
};

const char piece_J[] = {
  1,0,0,0,
  1,1,1,0,
  0,0,0,0,
  0,0,0,0,
  
  0,1,1,0,
  0,1,0,0,
  0,1,0,0,
  0,0,0,0,

  0,0,0,0,
  1,1,1,0,
  0,0,1,0,
  0,0,0,0,
  
  0,1,0,0,
  0,1,0,0,
  1,1,0,0,
  0,0,0,0,

};

const char piece_T[] = {
  0,1,0,0,
  1,1,1,0,
  0,0,0,0,
  0,0,0,0,

  0,1,0,0,
  0,1,1,0,
  0,1,0,0,
  0,0,0,0,
  
  0,0,0,0,
  1,1,1,0,
  0,1,0,0,
  0,0,0,0,

  0,1,0,0,
  1,1,0,0,
  0,1,0,0,
  0,0,0,0,

};

const char piece_S[] = {
  0,1,1,0,
  1,1,0,0,
  0,0,0,0,
  0,0,0,0,

  0,1,0,0,
  0,1,1,0,
  0,0,1,0,
  0,0,0,0,

  0,0,0,0,
  0,1,1,0,
  1,1,0,0,
  0,0,0,0,

  1,0,0,0,
  1,1,0,0,
  0,1,0,0,
  0,0,0,0,
};

const char piece_Z[] = {
  1,1,0,0,
  0,1,1,0,
  0,0,0,0,
  0,0,0,0,
  
  0,0,1,0,
  0,1,1,0,
  0,1,0,0,
  0,0,0,0,

  0,0,0,0,
  1,1,0,0,
  0,1,1,0,
  0,0,0,0,
  
  0,1,0,0,
  1,1,0,0,
  1,0,0,0,
  0,0,0,0,
};

const char piece_O[] = {
  1,1,0,0,
  1,1,0,0,
  0,0,0,0,
  0,0,0,0,
  
  1,1,0,0,
  1,1,0,0,
  0,0,0,0,
  0,0,0,0,
  
  1,1,0,0,
  1,1,0,0,
  0,0,0,0,
  0,0,0,0,
  
  1,1,0,0,
  1,1,0,0,
  0,0,0,0,
  0,0,0,0,
};

// Array com o ponteiro ou vetor de cada tipo de peça com suas orientações
const char *pieces[NUM_PIECE_TYPES] = {
  piece_S,
  piece_Z,
  piece_L,
  piece_J,
  piece_O,
  piece_T,
  piece_I,
};

// Cor das peças
const long piece_colors[NUM_PIECE_TYPES] = {
  0x009900, // verde S
  0xFF0000, // vermelho Z
  0xFF8000, // laranja L
  0x000044, // azul J
  0xFFFF00, // amarelo O
  0xFF00FF, // roxo T
  0x00FFFF,  // ciano I
};

/*STRAND_LENGTH é o número de pixels do painel
  LED_DATA_PIN é o pino que enviara os dados com o painel de leds
  Existem outros parametros que ficaram no padrão da biblioteca neopixel
*/

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRAND_LENGTH, LED_DATA_PIN);// Instancia um objeto que controla o painel de leds

int old_button=0;
// so arduino can tell when user moves sideways
int old_px = 0;
// so arduino can tell when user tries to turn
int old_i_want_to_turn=0;

// dados da peça caindo 
int piece_id;
int piece_rotation;
int piece_x;
int piece_y;

char piece_sequence[NUM_PIECE_TYPES];
char sequence_i=NUM_PIECE_TYPES;
// controla a velocidade de movimento
long last_move;
long move_delay;  // 
// controla a caida automatica da peça.
long last_drop;
long drop_delay;  

long last_draw;
long draw_delay;  // 60 fps

// array que guarda a posição das peças no painel, são 128 posiçoes de led
long grid[GRID_W*GRID_H];

// Essa função pinta uma posição e utilizando o sistema 
// O nosso plano cartesiano começa com o (x, y) = (0,0) do canto superior esquerdo do tabuleiro
// E cresce positivamente em x para a direita e em y para baixo
/*(0,0)
   ----------->+x (8 posições)
  |
  |
  |
  +y (16 posições)
*/
void pintar_pixel(int x,int y,long color) {
  int a = (GRID_H-1-y)*GRID_W; // (16-1-y)*8 = 120-y*8
  if( ( y % 2 ) == BACKWARDS ) {  
    // y%2 is false quando y é par - linhas 0,2,4,6. e o painel for em S
    a += x;// 120-y*8+x
  } else {
    // y%2 quando y é impar - linhas 1,3,5,7.
    a += GRID_W - 1 - x;// 120-y*8+8-1-x = 127-y*8-x
  }
  a%=STRAND_LENGTH;//
  strip.setPixelColor(a,color);//Pinta a cor na posição desejada
}


// grid contains the arduino's memory of the game board, including the piece that is falling.
/*
  Desenha o painel com base no vetor que contem os dados do painel, de cada led 
*/
void desenha_painel() {
  int x, y;
  //loop atualizando os pixels do painel com base no vetor GRID
  for(y=0;y<GRID_H;++y) {
    for(x=0;x<GRID_W;++x) {
      if(grid[y*GRID_W+x] != 0 ) {
        pintar_pixel(x,y,grid[y*GRID_W+x]);
      } else {
        pintar_pixel(x,y,0);
      }
    }
  }
  //Atualiza o painel
   delay(1);
   strip.show();
   delay(1);
}

/*
Escolhe uma peça do do tipo I, J, L, S, Z ou O
*/
void sorteia_peca() {
  if( sequence_i >= NUM_PIECE_TYPES ) {
    
    int i,j, k;
    for(i=0;i<NUM_PIECE_TYPES;++i) {
      do {
        // escolhe uma nova peca
        j = rand() % NUM_PIECE_TYPES;
        //verifica se ja esta na sequencia
        for(k=0;k<i;++k) {
          if(piece_sequence[k]==j) break;  // ja esta na sequencia
        }
      } while(k<i);
      //se n estiver na sequencia, a adiciona
      piece_sequence[i] = j;
    }
    
    sequence_i=0;
  }
  
  //pega a nova peca na sequencia
  piece_id = piece_sequence[sequence_i++];
  // começa a derrubar a peça do topo central do paienl
  piece_y=-4;  // -4 quadrados do topo do painel.
  piece_x=3;
  // sempre da mesma orientação
  piece_rotation=0;
}


void erase_piece_from_grid() {
  int x, y;
  
  const char *piece = pieces[piece_id] + (piece_rotation * PIECE_H * PIECE_W);
  
  for(y=0;y<PIECE_H;++y) {
    for(x=0;x<PIECE_W;++x) {
      int nx=piece_x+x;
      int ny=piece_y+y;
      if(ny<0 || ny>GRID_H) continue;
      if(nx<0 || nx>GRID_W) continue;
      if(piece[y*PIECE_W+x]==1) {
        grid[ny*GRID_W+nx]=0; 
      }
    }
  }
}


void add_piece_to_grid() {
  int x, y;
  
  const char *piece = pieces[piece_id] + (piece_rotation * PIECE_H * PIECE_W);
  
  for(y=0;y<PIECE_H;++y) {
    for(x=0;x<PIECE_W;++x) {
      int nx=piece_x+x;
      int ny=piece_y+y;
      if(ny<0 || ny>GRID_H) continue;
      if(nx<0 || nx>GRID_W) continue;
      if(piece[y*PIECE_W+x]==1) {
        grid[ny*GRID_W+nx]=piece_colors[piece_id];  
      }
    }
  }
}



/*
  Move para baixo tudo e remove a linha y
*/
void delete_row(int y) {

  score = score + 10;
 
  //Atualizar placar

  int x;
  for(;y>0;--y) {
    for(x=0;x<GRID_W;++x) {
      grid[y*GRID_W+x] = grid[(y-1)*GRID_W+x];
    }
  }
  // everything moved down 1, so the top row must be empty or the game would be over.
  for(x=0;x<GRID_W;++x) {
    grid[x]=0;
  }
}


void fall_faster() { 
  if(drop_delay > DROP_MINIMUM) drop_delay -= DROP_ACCELERATION;
}


void remove_full_rows() {
  int x, y, c;
  char row_removed=0;
  
  for(y=0;y<GRID_H;++y) {
    // count the full spaces in this row
    c = 0;
    for(x=0;x<GRID_W;++x) {
      if( grid[y*GRID_W+x] > 0 ) c++;
    }
    if(c==GRID_W) {
      // row full!
      delete_row(y);
      fall_faster();
    }
  }  
}


void try_to_move_piece_sideways() {
  // what does the joystick angle say
  int dx = map(analogRead(joystick_x),0,1023,512,-512);
  int new_px = 0;
  // is the joystick really being pushed?
  if(dx> JOYSTICK_DEAD_ZONE) {
    new_px=-1;
  }
  if(dx<-JOYSTICK_DEAD_ZONE) {
    new_px=1;
  }


  if(new_px!=old_px && piece_can_fit(piece_x+new_px,piece_y,piece_rotation)==1) {
    piece_x+=new_px;
  }
  old_px = new_px;
}

/*
  tenta rotacionar a peça
*/
void try_to_rotate_piece() {
  int i_want_to_turn=0;
  
  // joystick pra cima gira a peça
  int dy = map(analogRead(joystick_y),0,1023,512,-512);
  if(dy>JOYSTICK_DEAD_ZONE) i_want_to_turn=1;
  
  if(i_want_to_turn==1 && i_want_to_turn != old_i_want_to_turn) {
    
    int new_pr = ( piece_rotation + 1 ) % 4;
    
    if(piece_can_fit(piece_x,piece_y,new_pr)) {
     
      piece_rotation = new_pr;
    } else {
      // wall kick
      if(piece_can_fit(piece_x-1,piece_y,new_pr)) {
        piece_x = piece_x-1;
        piece_rotation = new_pr;
      } else if(piece_can_fit(piece_x+1,piece_y,new_pr)) {
        piece_x = piece_x+1;
        piece_rotation = new_pr;
      } 
    }
  }
  old_i_want_to_turn = i_want_to_turn;
}


/*
  Testa se a peça encaixa no espaço (x, y)
  Retorna 0 caso não e 1 para sim
*/
int piece_can_fit(int x,int y,int pr) {
  if( piece_off_edge(x,y,pr)||piece_hits_rubble(x,y,pr) )
  {
    return 0;
  } 
  return 1;
}


int piece_off_edge(int px,int py,int pr) {
  int x,y;
  const char *piece = pieces[piece_id] + (pr * PIECE_H * PIECE_W);
  
  for(y=0;y<PIECE_H;++y) {
    for(x=0;x<PIECE_W;++x) {
      int nx=px+x;
      int ny=py+y;
      if(ny<0) continue;  // 
      if(piece[y*PIECE_W+x]>0) {
        if(nx<0) return 1;  //esta no canto esquedo
        if(nx>=GRID_W ) return 1;  // esta no canto direito
      }
    }
  }
  
  return 0;  // dentro dos limites
}


int piece_hits_rubble(int px,int py,int pr) {
  int x,y;
  const char *piece = pieces[piece_id] + (pr * PIECE_H * PIECE_W);
  
  for(y=0;y<PIECE_H;++y) {    
    int ny=py+y;
    if(ny<0) continue;
    for(x=0;x<PIECE_W;++x) {
      int nx=px+x;
      if(piece[y*PIECE_W+x]>0) {
        if(ny>=GRID_H ) return 1;  // esta no canto inferior 
        if(grid[ny*GRID_W+nx]!=0 ) return 1;  // painel cheio nesta posicao
      }
    }
  }
  
  return 0;  //nao acertou
}


/*
  Gera uma animação no final do jogo
*/
void animacao_finaljogo()
{
  for(int i=0;i<STRAND_LENGTH;i++)
  {
    strip.setPixelColor(i, strip.Color(178, 34, 34));
  }
   strip.setPixelColor(55, strip.Color(150,150,150)); 
 
   strip.setPixelColor(74, strip.Color(150,150,150)); 
   strip.setPixelColor(77, strip.Color(150,150,150));
   strip.setPixelColor(83, strip.Color(150,150,150));  
   strip.setPixelColor(85, strip.Color(150,150,150)); 
   strip.setPixelColor(90, strip.Color(150,150,150)); 
   strip.setPixelColor(91, strip.Color(150,150,150)); 
   strip.setPixelColor(92, strip.Color(150,150,150)); 
   strip.setPixelColor(93, strip.Color(150,150,150)); 
   strip.setPixelColor(98, strip.Color(150,150,150)); 
   strip.setPixelColor(101, strip.Color(150,150,150)); 
   strip.setPixelColor(106, strip.Color(150,150,150)); 
   strip.setPixelColor(107, strip.Color(150,150,150)); 
   strip.setPixelColor(108, strip.Color(150,150,150)); 
   strip.setPixelColor(109, strip.Color(150,150,150)); 
   
   delay(1);
   strip.show();
   delay(1);
   
   if(!pause_onece)
   {
    myDFPlayer.play(1);  /* Le o primeiro arquivo no cartão */
    pause_onece = true;
    delay(1000);
   }

}



/*
  Pinta o painel de branco
*/
void painel_branco()
{
  for(int i=0;i<STRAND_LENGTH;i++){
    strip.setPixelColor(i, strip.Color(130,130,130)); 
    delay(1);
    strip.show();
    delay(3); 
  }
}

/*
  Rotina para o final de jogo
  Le o botao de fim de jogo e temporiza
*/
void game_over() {
  score = 0;//Reseta o score
  myDFPlayer.play(3);  /* Le o terceiro arquivo no cartão */
  game_over_loop_leds();
  delay(500);
  int x,y;

  long over_time = millis();
  animacao_finaljogo();
  currentMillis = millis();
  previousMillis = currentMillis;
  int led_number = 1;

  while(millis() - over_time < 8000) {

    currentMillis = millis();
    if(currentMillis - previousMillis >= 1000){
      previousMillis += 1000;
      
      strip.setPixelColor(55-led_number, strip.Color(150,150,150));  
     
      delay(1);
      strip.show();
      delay(1);
     
      led_number += 1; 
    }    

    // Se o start foi pressionado, gera animacao e  chama o setup para reiniciar o jogo
    if(!digitalRead(button_start)) {
      // reinicia
      myDFPlayer.play(2);  /* Le o segundo arquivo no cartão */
      painel_branco();
      delay(400);
      break;
    }
  }
  
  painel_branco();
  setup();
 
}

void game_over_loop_leds()
{
  for(int i=0;i<STRAND_LENGTH;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    strip.setPixelColor(i, strip.Color(0,150,0)); // Moderately bright green color.
   delay(1);
   strip.show();
   delay(10); // Delay for a period of time (in milliseconds).
  }
}
/*
  Tenta descer a peça 
*/
void try_to_drop_piece() {
  erase_piece_from_grid();  
  if(piece_can_fit(piece_x,piece_y+1,piece_rotation)) {
    piece_y++;  // move a peça para baixo
    add_piece_to_grid();
  } else {
    //acertou algo, coloca de novo
    add_piece_to_grid();
    remove_full_rows();
    if(game_is_over()==1) {
      game_over();
    }
    // escolhe uma nova peça
    sorteia_peca();
  }
}
/*
Verifica a escolha de caida rápida da peça.
*/
void caida_rapida() {
  int y = map(analogRead(joystick_y),0,1023,512,-512);//Lê o joystick vertical
  if(y<-JOYSTICK_DEAD_ZONE) {//Verifica se o jogador está com o joystick abaixo da zona de não reação do joystick
    // Se o jogador puxar para baixo constantemente, então a peça é acelerada para baixo
    try_to_drop_piece();//Joga a peça pra baixo
  }
}

/*
  Reage ao jogador  utilizando funcoes por pooling
*/
void react_to_player() {
  erase_piece_from_grid();//apaga a peça do painel
  try_to_move_piece_sideways();//tenta mover a peça para os lados
  try_to_rotate_piece();//tenta rotacionar a peça
  add_piece_to_grid();//adiciona a peça ao painel
  caida_rapida();//tenta jogar a peça pra baixo rapido
}


// can the piece fit in this new location
int game_is_over() {
  int x,y;
  const char *piece = pieces[piece_id] + (piece_rotation * PIECE_H * PIECE_W);
  
  for(y=0;y<PIECE_H;++y) {
    for(x=0;x<PIECE_W;++x) {      
      int ny=piece_y+y;
      int nx=piece_x+x;
      if(piece[y*PIECE_W+x]>0) {
        if(ny<0) return 1;  // yes: off the top!
      }
    }
  }
  
  return 0;  // not over yet...
}



void setup() {
  /**************       CONFIGURAÇÕES DE AUDIO        *****************************/
  mySoftwareSerial.begin(9600, SERIAL_8N1, 16, 17);  /* velocidade, tipo de comunicação, pinos RX, TX */
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Iniciando módulo DFPlayer ..."));  

  delay(1000);//leva entre 1 a 3 segundos para módulo inicializar

  if (!myDFPlayer.begin(mySoftwareSerial, false))
  {  /* Verifica o funcionamento do módulo.  Se não for capaz de identificar o módulo */                                                                
    Serial.println(myDFPlayer.readType(), HEX);
    Serial.println(F("Erro ao iniciar, verifique:"));
    Serial.println(F("1. A conexao do modulo."));
    Serial.println(F("2. Se o SD card foi inserido corretamente."));
    
  }

  Serial.println(F("DFPlayer Mini online.")); /* Ao identificar o funcionamento do módulo */

  myDFPlayer.setTimeOut(500); /* Ajusta o tempo de comunicação para 500ms */
 
  /* ----Ajuste do Volume---- */
  myDFPlayer.volume(10);  /* Volume de 0 a 30. */
  myDFPlayer.volumeUp();  /* Volume Up */

  /*----Ajusta o Equalizador---- */
  myDFPlayer.EQ(0); /*  0 = Normal, 1 = Pop, 2 = Rock, 3 = Jazz, 4 = Classic, 5 = Bass */

  /*----Define o dispositivo---- */
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  /*----Quantidade de Arquivos---- */
  Serial.print(F("Numero de arquivos no cartao SD: "));
  Serial.println(myDFPlayer.readFileCounts(DFPLAYER_DEVICE_SD));
  myDFPlayer.play(4);  /* Le o quarto  arquivo no cartão , que será a música longa*/
  
  pinMode(button_pause,INPUT_PULLUP);//Ativa a entrada do pause com nível alto
  pinMode(button_start,INPUT_PULLUP);//Ativa a entrada do start com nível alto
  delay(1);

  //cartao - esperado (original)
  //4     -     1   (musica que toca o jogo todo)
  //3     -     4   ()
  //2     -     3
  //1     -     2  

/************** FIM DAS CONFIGURAÇÕES DE AUDIO *****************************/

  // Inicialize o ADC com uma resolução de 10 bits.
  analogReadResolution(10);
  
  strip.begin();//Inicializa os leds
 
  delay(1);
  strip.show();
  delay(1);
  
  pinMode(JOYSTICK_PIN,INPUT);//define o o pino do botão do joystick
  digitalWrite(JOYSTICK_PIN,HIGH);
  
  // Reseta o vetor que guarda o estado do painel
  for(int i=0;i<GRID_W*GRID_H;++i) {
    grid[i]=0;
  }
  
  
  randomSeed(analogRead(joystick_y)+analogRead(2)+analogRead(3));
  
  // escolhe uma peca
  sorteia_peca();
  
  move_delay=INITIAL_MOVE_DELAY;
  drop_delay=INITIAL_DROP_DELAY;
  draw_delay=INITIAL_DRAW_DELAY;

  // start the game clock after everything else is ready.
  last_draw = last_drop = last_move = millis();
}




//Usado para o som de pause
void waitMilliseconds(uint16_t msWait)
{
  uint32_t start = millis();
  
  while ((millis() - start) < msWait)
  {
    // calling mp3.loop() periodically allows for notifications 
    // to be handled without interrupts
   // mp3.loop(); 
    delay(1);
  }
}

void animacao_pause()
{  
   for(int i=0;i<STRAND_LENGTH;i++)
   {
    strip.setPixelColor(i, strip.Color(0,0,0)); 
   }
   
   strip.setPixelColor(53, strip.Color(150,150,150)); 
   strip.setPixelColor(58, strip.Color(150,150,150)); 
   strip.setPixelColor(66, strip.Color(150,150,150)); 
   strip.setPixelColor(67, strip.Color(150,150,150)); 
   strip.setPixelColor(68, strip.Color(150,150,150)); 
   strip.setPixelColor(69, strip.Color(150,150,150)); 
   strip.setPixelColor(74, strip.Color(150,150,150)); 
   strip.setPixelColor(77, strip.Color(150,150,150)); 
   strip.setPixelColor(82, strip.Color(150,150,150)); 
   strip.setPixelColor(83, strip.Color(150,150,150)); 
   strip.setPixelColor(84, strip.Color(150,150,150)); 
   strip.setPixelColor(85, strip.Color(150,150,150)); 
   
   
   delay(1);
   strip.show();
   delay(1);
   
   
   if(!pause_onece)
   {
   //************** FIM DAS CONFIGURAÇÕES DE AUDIO *****************************/.playMp3FolderTrack(2);  // engine start + submarine sound
    pause_onece = true;
    delay(1000);
   }
}

void loop() {

  long t = millis();//Anota o tempo corrente do loop

  if(!Pause)//Se o jogo não estiver pausado, continuamos o jogo
  { //Jogo não pausado
    if(!digitalRead(button_pause) && !pause_pressed)
    {
      Pause = !Pause;
      pause_pressed = true;
      pause_onece = false;
    }
    if(digitalRead(button_pause) && pause_pressed)
    {      
      pause_pressed = false;
    }
    
    // Determina uma diferença de tempo Delta t entree o ultimo movimento de peça e o tempo atual, se o tempo for maior que o tempo de movimento, 
    // precisamos reagir a entrada do jogador e anotar o tempo em que reagimos para a mesma operação futura
    if(t - last_move > move_delay ) {
      last_move = t;
      react_to_player();
    }
    
    // ...and drops the falling block at a different speed.
    if(t - last_drop > drop_delay ) {
      last_drop = t;//guarda o tempo em que estamos deixando cair uma peça
      try_to_drop_piece();//derruba uma peça no tabuleiro
    }
    
    // Redesenha o painel com base no tempo de redesenho definido anteriormente
    if(t - last_draw > draw_delay ) {
      last_draw = t;//guarda o tempo em que estamos redesenhado o painel
      desenha_painel();//redesenha o painel
    }

  }else{
    //Jogo pausado
    if(!digitalRead(button_pause) && !pause_pressed)//Se botão de pause pressionado = 0V e o botão foi pressionado anteriormente
    {                                               //Invertemos o estado de Pause 
      Pause = !Pause;
      pause_pressed = true;
       myDFPlayer.play(4);  /* Le o primeiro arquivo no cartão */
    }
    if(digitalRead(button_pause) && pause_pressed)
    { //Se o    
      pause_pressed = false;
    }
    animacao_pause();//jogo pausado
    delay(1);
  }
}


