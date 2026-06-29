#include <Arduino.h>
#include "BasicStepperDriver.h"
#include <TimerOne.h>

// pinos para controle do tamanho do passo
#define MS1 4
#define MS2 3
#define MS3 2

// Todos os botões serão pull-up.

#define enableMotor  8 // pino para liberar (desligar) ou energizar o motor de passo (gerenciar consumo de energia)

#define pinB1 9 // Move rapido AR no sentido do TRACKING
#define pinB2 10 // Move rapido AR no outro sentido
#define pinB3 11 // Habilita o modo TRACKING
#define pinB4 12 // Seleciona sentido de rotação no modo TRACKING

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 60

// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 1=full step, 2=half step etc.
#define MICROSTEPS 1

// All the wires needed for full functionality
#define DIR 6
#define STEP 7

//#define Periodo 314000 // microsegundos / passo com o Celestron Powerseeker
//#define Periodo 250000 // microsegundos / passo com o Toya SkyMaster

//#define Periodo 249000 // Estava assim desde o início de 2026 até o dia 04_06_2026, mas realmente nunca tinha feito um ajuste fino até então.
//#define Periodo 244500 // tracking de estrelas e céu profundo
#define Periodo 252931 // tracking da Lua, considerando uma relação de 15 / 14.5 em relação ao valor para o tracking das estrelas.

// 2-wire basic config, microstepping is hardwired on the driver
BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);

int atraso = 10;

bool tracking = false, tracking_anterior = false;

bool statusB1 = true;
bool statusB2 = true;
bool statusB3 = true;
bool statusB4 = true;

bool statusB1_anterior = true;
bool statusB2_anterior = true;
bool statusB3_anterior = true;
bool statusB4_anterior = true;

bool sentido = true; // true = anti-horario e false = horario
// No telescopio do campus, o sentido ficou invertido, sendo necessario alterar para false.

int step = 1;

void MotorLento() // 1/16 de passo, menor velocidade
{
  Serial.println("Motor Lento");
  digitalWrite(MS1, 1);
  digitalWrite(MS2, 1);
  digitalWrite(MS3, 1);
}

void MotorRapido() // passo simples, maior velocidade
{
  Serial.println("Motor Rapido");
  digitalWrite(MS1, 0);
  digitalWrite(MS2, 0);
  digitalWrite(MS3, 0);
}

void MotorNormal() // 1/4 passo, velocidade intermediaria
{
  Serial.println("Motor Normal");
  digitalWrite(MS1, 0);
  digitalWrite(MS2, 1);
  digitalWrite(MS3, 0);
}

void SendStep()
{
  stepper.move(step);
}

void setup()
{
    Serial.begin(9600);

    pinMode(pinB1, INPUT_PULLUP);
    pinMode(pinB2, INPUT_PULLUP);
    pinMode(pinB3, INPUT_PULLUP);
    pinMode(pinB4, INPUT_PULLUP);

    pinMode(MS1, OUTPUT);
    pinMode(MS2, OUTPUT);
    pinMode(MS3, OUTPUT);

    pinMode(enableMotor, HIGH); // Lógica invertida: HIGH = motor desligado (liberado)

    MotorRapido();

    stepper.begin(RPM, MICROSTEPS);

    Serial.println("SETUP OK");
}

void loop()
{
    statusB1 = digitalRead(pinB1);
    statusB2 = digitalRead(pinB2);
    statusB3 = digitalRead(pinB3);
    statusB4 = digitalRead(pinB4);

    if(statusB1 == false && statusB1_anterior == true) // Se pressionou o botão B1, ativa o motor
    {    
        digitalWrite(enableMotor, LOW); // Lógica invertida: LOW = ativar/energizar motor
    }

    if(statusB2 == false && statusB2_anterior == true) // Se pressionou o botão B2, ativa o motor
    {    
        digitalWrite(enableMotor, LOW);
    }
    
    if(statusB1 == false) // Gira rapidamente eixo da AR no sentido do TRACKING
    {
      //Serial.println("Move 1 no sentido do tracking");
      
        stepper.move(step);

      delay(30); // estava 10 ms
    }

    if(statusB2 == false) // Gira rapidamente eixo da AR no outro sentido
    {
      //Serial.println("Move 1 no sentido oposto ao tracking");
      
        stepper.move(-step);

      delay(30);
    }
  
 
    /*if(statusB4 == false && statusB4_anterior == true)
    {
      Serial.println("B4: sentido motor");
      sentido = !sentido; // Inverte sentido do motor
      step = -step; // Inverte sinal do step
    }*/

    if (statusB3 == false && statusB3_anterior == true)
    {
      Serial.println("B3 tracking ON/OFF");
      tracking = !tracking; // Habilita ou desabilita o modo TRACKING

      // Se ativou o tracking, energiza motor. Caso contrário, desliga para destravar o eixo.
      if (tracking)
        {
          digitalWrite(enableMotor, LOW);
          Serial.println("EnableMotor = LOW");
        }
        
      else
      {
        digitalWrite(enableMotor, HIGH);
        Serial.println("EnableMotor = HIGH");
      }
        
    }
    
    // Se habilitou o modo TRACKING
    if(tracking == true && tracking_anterior == false)
    {
      Serial.println("TRACKING ON");

      digitalWrite(enableMotor, LOW);

      MotorNormal();

      // Habilita interrupção para comandar o motor
      Timer1.initialize(Periodo);
      Timer1.attachInterrupt(SendStep); // SendStep é a função que comanda o motor de passo no modo TRACKING
    }

    // Se desabilitou o modo TRACKING
    if(tracking == false && tracking_anterior == true)
    {
      Serial.println("TRACKING OFF");
      MotorRapido();

      // Desabilita interrupção para comandar o motor
      Timer1.detachInterrupt(); // SendStep é a função que comanda o motor de passo no modo TRACKING
    }

    statusB1_anterior = statusB1;
    statusB2_anterior = statusB2;
    statusB3_anterior = statusB3;
    statusB4_anterior = statusB4;
    tracking_anterior = tracking;

}
