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

int manage_bt, mode_bt;

int cnt_l = 0, cnt_r = 0, cnt_sw = 0, cnt_st = 0;

int manage_val, mode_val, relay_val;
bool left_flg = true, right_flg = true;

long dist = 10;
long pos = 0;
long targetPos;
int leftPace = -2000;
int rightPace = 2000;
GStepper<STEPPER2WIRE> stepper(8, STEP, DIR, EN);               // драйвер step-dir + пин enable
void setup() {
//  Timer0.setPeriod(1000);
//  Timer0.enableISR();
  analogReference(INTERNAL);
  Serial.begin(9600);  
  stepper.setRunMode(KEEP_SPEED);
  stepper.setCurrent(0);
  stepper.setSpeed(0);
  stepper.enable();
  targetPos = round(dist/0.062);
  pinMode(13, OUTPUT);
}
//
//ISR(TIMER0_B) {
//  stepper.tick(); // тикаем тут
//}

void loop() {
stepper.tick();
manage_val = analogRead(MANAGE_BT);
pos = stepper.getCurrent();
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
  else if ((manage_val > 560) && (manage_val < 610)) {
   cnt_r = 0;
   cnt_st = 0;
   cnt_l = 0;
   cnt_sw ++;
   if (cnt_sw >= 5){
    cnt_sw = 0; 
    manage_bt = 1; //концевик
    left_flg = true;
    right_flg = true;
  }
}
else {
   cnt_r = 0;
   cnt_st = 0;
   cnt_l = 0;
   cnt_sw = 0;
}

  if ((manage_bt == 0) || (manage_bt == 1)){
    stepper.setSpeed(0);
    digitalWrite(13, LOW);
  }

if ((manage_bt == 2) && (left_flg)){
        digitalWrite(13, HIGH);
        left_flg = false;
        stepper.setSpeed(rightPace);
        //Serial.println("right");
   }
if ((manage_bt == 3) && (right_flg)){
        digitalWrite(13, HIGH);
        right_flg  = false;
        stepper.setSpeed(leftPace);
        //Serial.println("left");
      }
//Serial.println(cnt_l);
}
