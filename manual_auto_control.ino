#include "GyverTimers.h"
//подключаем все блоки кнопок
#define SPEED_BT A1
#define MODE_BT A6
#define MANAGE_BT A0
//входы с блока cassy
#define REL_CAS A3
#define ANA_CAS A2
//пины для управления tmc2208
#define DIR 8
#define STEP 9
#define EN 10

#define STEP_ROUND 220*8 // число микрошагов в одном обороте
#define ROUND_DIST 8.7 // число мм в одном обороте 

int SPEED = 50; //на один микрошаг -- 2*SPEED микросекунд

int manage_bt, mode_bt; //переменные для выбора режима работы
//mode: 0 - manual
//      1 - auto
//      2 - not choosen
// manage: 0 - stop
//         1 - limiter switch
//         2 - left
//         3 - right
//         4 - not choosen
int manage_val, mode_val, relay_val; //переменные для считывания напряжения на аналоговом входе

//переменные для создания посылки в cassy
long pos = 0;
int pack[10];
int posVal;

float dist = 1; //переменная для хранения перемещения в мм
float deg = dist * 22.8; //переменная для хранения угла поворота в сотых градуса
long num = round(deg/0.18);

volatile int counter = 9;

//функция, делающая один микрошаг
void ONE_STEP(int pace) {
  digitalWrite(EN, LOW);
  digitalWrite(STEP, HIGH);
  delayMicroseconds(pace);
  digitalWrite(STEP, LOW);
  delayMicroseconds(pace);
  //digitalWrite(EN, HIGH);
}
//функция, выполняющая один микрошаг влево
void GO_LEFT(int pace) {
  if (manage_bt != 1){
  digitalWrite(DIR, HIGH);
  ONE_STEP(pace);
  pos--;
  }
  //Serial.println("left");
}
//функция, выполняющая один микрошаг вправо
void GO_RIGHT(int pace) {
  digitalWrite(DIR, LOW);
  ONE_STEP(pace);
  pos++;    
  //Serial.println("right");
}

void setup() {
  //analogReference(INTERNAL);
  // put your setup code here, to run once:
  attachInterrupt(0, packageSetup, CHANGE);
  
  Serial.begin(9600);

  Timer2.setPeriod(25);
  //пин для отправки данных в cassy 
  pinMode(4, OUTPUT);
  
  PORTD |= 1 << 4;
  
  pinMode(DIR, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(EN, OUTPUT);
  //digitalWrite(EN, HIGH);
  //digitalWrite(DIR, LOW);
  manage_bt = 0;
  mode_bt = 0;

//  while ((analogRead(MANAGE_BT) < 570) || (analogRead(MANAGE_BT) > 600)){
//    GO_LEFT(SPEED);
//  }
//едем влево, пока не упремся в концевик. Во избежание дребезга - ждем тройного "срабатывания" концевика
  int cnt = 0;
  while (true){
   manage_val = analogRead(MANAGE_BT);
   if ((manage_val > 570) && (manage_val < 600)){
   cnt++; 
   if (cnt >= 3){
   break; 
   }
   }
   else cnt = 0;
    GO_LEFT(SPEED);
  }
  pos = 0;
  //Serial.println("Концевик!!");
  delay(1000);
  //едем вправо на 10 см
  while (pos <= 20400){
    GO_RIGHT(SPEED);
  }
  delay(1000);
}
//функция для подготовки массива, в котором собирается очередность и длительность импульсов для отправки в cassy
void packageSetup(){
  Timer2.disableISR(CHANNEL_B);
//  dist = pos * ROUND_DIST/STEP_ROUND;
//  deg = dist * 22.92;
//  num = round(deg / 0.18);
  num = round(pos*0.062); 
// num = 20;
  for (int i = 0; i < 4; i++){
    posVal = bitRead(num, 0)*(1<<0)+bitRead(num, 1)*(1<<1)+bitRead(num, 2)*(1<<2)+bitRead(num, 3)*(1<<3);
    pack[2*i+1] = posVal + 7;
    pack[2*i+2] = posVal + 7;
    num >>= 4;
  }
  pack[9] = 7;
  pack[0] = 1;
  counter = 9;
  Timer2.enableISR(CHANNEL_B);
}
//по прерыванию отправляется посылка, если есть, что отправлять
ISR(TIMER2_B){
  if (counter >= 0){
    if (pack[9 - counter] > 0){
      pack[9 - counter] --;
    }
    else{
      PORTD = PORTD ^ 1 << 4;
      counter --;
    }
  }
  else{
    Timer2.disableISR(CHANNEL_B);
    PORTD |= 1 << 4;
  }
}

void loop() {
  //вначале каждого цикла считываем значения с кнопок режима и кнопок управления
  manage_val = analogRead(MANAGE_BT);
  mode_val = analogRead(MODE_BT);
  if ((mode_val > 920) && (mode_val < 945)) mode_bt = 1; //режим управления с cassy
  if ((mode_val > 850) && (mode_val < 880)) mode_bt = 0; //режим ручного управления
  
  if ((manage_val > 670) && (manage_val < 690)) manage_bt = 0; //кнопка стоп
  if ((manage_val > 570) && (manage_val < 600)) manage_bt = 1; //концевик
  

  if (manage_bt == 1) digitalWrite(EN, HIGH);
  
  if (mode_bt == 0){
      if ((manage_val > 770) && (manage_val < 800)) manage_bt = 3; //едем вправо
      if ((manage_val > 920) && (manage_val < 940)) manage_bt = 2; //едем влево
      SPEED = 205 - analogRead(SPEED_BT)/5;
      if (manage_bt == 0) digitalWrite(EN, HIGH);
      if (manage_bt == 2){
        GO_LEFT(SPEED);
      }
      if (manage_bt == 3){
        GO_RIGHT(SPEED);
      }
      mode_bt = 2;
  }

  if (mode_bt == 1){
    //if (manage_bt < 2) digitalWrite(EN, HIGH);
    if (manage_val == 0) manage_bt = 4;
    //добавлен порог в 15 единиц, дабы избежать дребезгов
    if (analogRead(ANA_CAS) > 15){
      relay_val = analogRead(REL_CAS); 
      SPEED = 198 - analogRead(ANA_CAS)/5;
      if (relay_val == 0){
        GO_LEFT(SPEED);
    }
      if (relay_val != 0){
        GO_RIGHT(SPEED);
      }
    }
    mode_bt = 2;
  }
}
