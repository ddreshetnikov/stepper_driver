// демо - основные возможности библиотеки
#include <GyverStepper.h>
#include <GyverTimers.h>
#define DIR 8
#define STEP 9
#define EN 10

//подключаем все блоки кнопок
#define SPEED_BT A1
#define MODE_BT A6
#define MANAGE_BT A0

#define REL_CAS A3
#define ANA_CAS A2

#define STEP_ROUND 220*8 // число микрошагов в одном обороте
#define ROUND_DIST 8.7 // число мм в одном обороте 

int manage_bt, mode_bt;

int cnt_l = 0, cnt_r = 0, cnt_sw = 0, cnt_st = 0;

int manage_val, mode_val, relay_val;

bool left_flg = true, right_flg = true;

long pos = 0;
int pack[10];
int posVal;

volatile int counter = 9;

long num = round(pos*0.063); 

int pace = 2000;
GStepper<STEPPER2WIRE> stepper(8, STEP, DIR, EN);               // драйвер step-dir + пин enable

void packageSetup(){
  Timer2.disableISR(CHANNEL_B);
//  dist = pos * ROUND_DIST/STEP_ROUND;
//  deg = dist * 22.92;
//  num = round(deg / 0.18);
  pos = stepper.getCurrent();
  //pos = 0;
  num = round(pos*0.062); 
  //num = 20;
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

void setup() {
  attachInterrupt(0, packageSetup, CHANGE);
  Timer2.setPeriod(25);
  Timer2.enableISR(CHANNEL_B);
//  Timer1.setPeriod(100);
//  Timer1.enableISR(CHANNEL_B);

  pinMode(4, OUTPUT);
  PORTD |= 1 << 4;
  
  pinMode(DIR, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(EN, OUTPUT);
  
  analogReference(INTERNAL);
  Serial.begin(9600);  
  stepper.setRunMode(KEEP_SPEED);
  stepper.setCurrent(0);
  stepper.setSpeed(0);
  stepper.enable();
  pinMode(13, OUTPUT);
}
//
//ISR(TIMER1_B) {
//  //stepper.tick();
//}

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
//pos = stepper.getCurrent();
stepper.tick();
manage_val = analogRead(MANAGE_BT);
mode_val = analogRead(MODE_BT);

if (mode_val > 915) mode_bt = 1;
if ((mode_val > 840) && (mode_val < 880)) mode_bt = 0;


if ((manage_val > 760) && (manage_val < 810)){
  cnt_l = 0;
  cnt_sw = 0;
  cnt_st = 0;
  cnt_r ++;
  if (cnt_r >= 5){
    cnt_r = 0; 
    manage_bt = 3; //едем вправо
    left_flg = true;
  }
}
else if ((manage_val > 910) && (manage_val < 950)){
  cnt_r = 0;
  cnt_sw = 0;
  cnt_st = 0;
  cnt_l ++;
  if (cnt_l >= 5){
    cnt_l = 0; 
    manage_bt = 2; //едем влево
    right_flg = true;
  }
}
  else if ((manage_val > 660) && (manage_val < 700)) {
   cnt_r = 0;
   cnt_sw = 0;
   cnt_l = 0;
   cnt_st ++;
   if (cnt_st >= 5){
    cnt_st = 0; 
    manage_bt = 0; //кнопка стоп
    left_flg = true;
    right_flg = true;
  }
}
  else if ((manage_val > 540) && (manage_val < 620)) {
   cnt_r = 0;
   cnt_st = 0;
   cnt_l = 0;
   cnt_sw ++;
   if (cnt_sw >= 5){
    cnt_sw = 0; 
    manage_bt = 1; //концевик
    left_flg = true;
    right_flg = true;
    
    stepper.setCurrent(0);
    pos = 0;
  }
}
else {
   cnt_r = 0;
   cnt_st = 0;
   cnt_l = 0;
   cnt_sw = 0;
}

  if ((manage_bt == 0) || (manage_bt == 1)){
    //stepper.setSpeed(0);
    stepper.disable();
    stepper.brake();
    digitalWrite(13, LOW);
  }
if (mode_bt == 0){
pace = 2.5*analogRead(SPEED_BT);
if ((manage_bt == 2) && (left_flg)){
        stepper.enable();
        digitalWrite(13, HIGH);
        left_flg = false;
        Timer2.disableISR(CHANNEL_B);
        stepper.setSpeed(pace);
        Timer2.enableISR(CHANNEL_B);
        //Serial.println("right");
   }
if ((manage_bt == 3) && (right_flg)){
        stepper.enable();
        digitalWrite(13, HIGH);
        right_flg  = false;
        Timer2.disableISR(CHANNEL_B);
        stepper.setSpeed(-1*pace);
        Timer2.enableISR(CHANNEL_B);
        //Serial.println("left");
      }
      mode_bt = 2;
  }
  
//
//    if (mode_bt == 1){
//    //if (manage_bt < 2) digitalWrite(EN, HIGH);
//    if (manage_val == 0) manage_bt = 4;
//    if (analogRead(ANA_CAS) > 30){
//      relay_val = analogRead(REL_CAS); 
//      SPEED = 2*analogRead(ANA_CAS);
//      if (relay_val == 0){
//        GO_LEFT(SPEED);
//    }
//      if (relay_val != 0){
//        GO_RIGHT(SPEED);
//      }
//    }
//    mode_bt = 2;
//  }
  
//Serial.println(pos);
}
