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
  //analogReference(INTERNAL);
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
if ((manage_val > 770) && (manage_val < 800)){ 
  manage_bt = 3; //едем вправо
  left_flg = true;
}
if ((manage_val > 920) && (manage_val < 940)){ 
  manage_bt = 2; //едем влево
  right_flg = true;
}

  if ((manage_val > 670) && (manage_val < 690)) {
    manage_bt = 0; //кнопка стоп
    left_flg = true;
    right_flg = true;
  }
  if ((manage_val > 570) && (manage_val < 600)) {
    manage_bt = 1; //концевик
    left_flg = true;
    right_flg = true;
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
//Serial.println(pos);
}
