#include <TimerOne.h>
#include <VirtualWire.h>

const int led_pin = 13;
const int transmit_pin = 12;

/* Counter and ISR variables */
unsigned char isrTick8, isrCountDownTick8 = 1;
unsigned char segment1,segment2,segment3,segment4;
unsigned char clockTimer8, countDownTick;

/* loop variables */
unsigned char dayTime[2];
unsigned long oldTime;


byte count = 1;
unsigned char sendPeriod;


void setup() {
  delay(1000);
  Serial.begin(9600);  // Debugging only
  Serial.println("setup");

  // put your setup code here, to run once:
  vw_set_tx_pin(transmit_pin);
  pinMode(13, OUTPUT);
  // pinMode(transmit_pin, OUTPUT);
  vw_setup(2000);
//  Timer1.initialize(102500);
//  Timer1.attachInterrupt(ClockTick);
  isrTick8 = 0;
  dayTime[0] = 56;
  dayTime[1] = 14;
}



void loop() {
//    unsigned char loopTimer;


  // put your main code here, to run repeatedly:
  char msg[1]; // = {'#'};
//  char msg[7] = {'h','e','l','l','o',' ','#'};

  if (millis() - oldTime > 100){ /* 100 ms task*/
    oldTime = oldTime + 100;  
    ClockTick();
  }

//  loopTimer = clockTimer8;
  if (clockTimer8 > 59){
    clockTimer8 = 0; //clockTimer8 - 60;
    if (dayTime[0] < 59){
      dayTime[0]++;
    }
    else if (dayTime[1] < 23){
      dayTime[0] = 0;
      dayTime[1]++;
    }
    else {
      dayTime[0] = 0;
      dayTime[1] = 0;
    }
  }
  
  if(!sendPeriod) {
    sendPeriod = 100;
    msg[0] = 1;
    msg[1] = dayTime[1];
    msg[2] = dayTime[0];
    msg[3] = clockTimer8;
    msg[4] = isrTick8;
    Serial.print(msg[0], DEC);
    Serial.print(' ');
    Serial.print(msg[1], DEC);
    Serial.print(' ');
    Serial.print(msg[2], DEC);
    Serial.print(' ');
    Serial.print(msg[3], DEC);
    Serial.print(' ');
    Serial.print(msg[4], DEC);
    Serial.println();
    digitalWrite(led_pin, HIGH); // Flash a light to show transmitting
    vw_send((uint8_t *)msg, 5);
    vw_wait_tx(); // Wait until the whole message is gone
    digitalWrite(led_pin, LOW);
    
  } else sendPeriod--;

  delay(10);
}

void ClockTick(void){
  char i;
  isrTick8++;
  isrCountDownTick8--;

  if(isrTick8 > 9){               /* 1 sec routine */
  isrTick8 = 0;
  clockTimer8++;
  }

  if(!isrCountDownTick8){
    isrCountDownTick8 = 10;
    if (countDownTick>136) {
      countDownTick--;
      segment1 = 9;
      segment2 = 9;
      segment3 = 9;
      segment4 = 9;
    }
    else if (countDownTick>121) {
      countDownTick--;
      i = countDownTick - 121;
      segment1 = 0;
      segment2 = 0;
      segment3 = i / 10;
      segment4 = i % 10;
    }
    else if (countDownTick) {
      countDownTick--;
      segment1 = 0;
      if (countDownTick>99){
        segment2 = 1;
        segment3 = (countDownTick - 100) / 10;
      }
      else{
        segment2 = 0;
        segment3 = countDownTick / 10;
      }
      segment4 = countDownTick % 10;
    }
  }
}
