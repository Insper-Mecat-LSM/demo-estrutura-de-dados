#include "mbed.h"
#include "Motor.h"

// DECLARAÇÃO DOS PINOS NECESSARIOS

InterruptIn FC_INICIAL(D5);
InterruptIn FC_FIM(D6);

DigitalOut enable_motor_1(PC_8);
DigitalOut step(PB_11);
Motor motor1(PA_12);

InterruptIn CONFIRMA_POSICAO(D13);
AnalogIn JOYSTICK(A0);

// DECLARAÇÃO DAS VARIAVEIS NECESSARIAS
Timer timer;
Timer debounce;

bool flag_fc = 1;
int tempo = 800;

// DECLARAÇÃO DA STRUCT UTILIZADA NO CODIGO
struct Position
{
  int pos_init;
  int pos_end;
  int pos_middle;
};

// DECLARAÇÃO DO ENUM UTILIZADO NO CODIGO
enum ESTADOS
{
  BUSCA_ZERO = 1,
  BUSCA_PONTO_MAX = 2,
  BUSCA_MEIO = 3,
  JOG_MANUAL = 4,
};

int passos = 0;
int sinal_joystick;
ESTADOS estado = BUSCA_ZERO;

Position motor1_positions;

// Declarando o inicio das funções utilizadas
void toggle_fc_init();
void toggle_fc_end();

// Criando as funções
void toggle_fc_init()
{
  // LOGICA DE DEBOUNCE
  if (debounce.read_ms() > 50)
  {
    // LOGICA DE INTERRUPÇÃO DO FC
    if (flag_fc == 1)
    {
      flag_fc = 0;
      motor1_positions.pos_init = 0;
      passos = 0;
      estado = BUSCA_PONTO_MAX;
    }
    printf("Estado fc: %d \n \r", flag_fc);
  }

  debounce.reset();
}

void toggle_fc_end()
{
  // LOGICA DE DEBOUNCE
  if (debounce.read_ms() > 50)
  {
    // LOGICA DE INTERRUPÇÃO DO FC
    if (flag_fc == 0)
    {
      flag_fc = 1;
      motor1_positions.pos_end = passos;
      motor1_positions.pos_middle = passos / 2;
      estado = BUSCA_MEIO;
    }

    printf("Estado fc: %d \n \r", flag_fc);
  }

  debounce.reset();
}

int main()
{
  debounce.start();
  FC_INICIAL.fall(&toggle_fc_init);
  FC_FIM.fall(&toggle_fc_end);

  enable_motor_1 = 1;
  step = 1;

  while (1)
  {
    switch (estado)
    {
    case BUSCA_ZERO:
      enable_motor_1 = 0;
      step = !step;
      wait_us(tempo);
      motor1.gira_motor_sentido_horario();
      break;
    case BUSCA_PONTO_MAX:
      enable_motor_1 = 0;
      step = !step;
      wait_us(tempo);
      motor1.gira_motor_sentido_antihorario();
      passos = passos + 1;
      break;
    case BUSCA_MEIO:
      enable_motor_1 = 0;
      step = !step;
      wait_us(tempo);
      motor1.gira_motor_sentido_horario();
      passos = passos - 1;
      if (passos == motor1_positions.pos_middle)
      {
        enable_motor_1 = 1;
        wait_ms(2000);
        estado = JOG_MANUAL;
      }
      break;
    case JOG_MANUAL:
      while (1)
      {
        enable_motor_1 = 1;
        sinal_joystick = JOYSTICK.read() * 1000;
        if (sinal_joystick > 800)
        {
          enable_motor_1 = 0;
          step = !step;
          wait_us(tempo);
          motor1.gira_motor_sentido_antihorario();
        }
        if (sinal_joystick < 300)
        {
          enable_motor_1 = 0;
          step = !step;
          wait_us(tempo);
          motor1.gira_motor_sentido_horario();
        }
      }
      break;
    }
  }
}
