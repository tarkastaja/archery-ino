
#include <TimerOne.h>
#include <VirtualWire.h>

const int SEG_1_LE_pin = 2;
const int SEG_2_LE_pin = 3;
const int SEG_3_LE_pin = 4;
const int SEG_4_LE_pin = 5;
const int transmit_pin = 6;
const int receive_pin = 7;
const int SEG_DATA1 = 8;
const int SEG_DATA2 = 9;
const int SEG_DATA3 = 10;
const int SEG_DATA4 = 11;
const int led_pin = 13;

unsigned char isrTick8 = 1, isrCountDownTick8 = 1;
unsigned char segment1,segment2,segment3,segment4;
unsigned char clockTimer8, countDownTick;
unsigned char dayTime[2];
unsigned char displayMode;

char clockJitter;
char isrTick8add;


void setup()
{
  delay(1000);
  Serial.begin(9600);  // Debugging only
  Serial.println("setup");

  // Initialise VirtualWire
  vw_set_rx_pin(receive_pin);     
  vw_set_ptt_inverted(true);     // Required for DR3100
  vw_setup(2000);                // Bits per sec
  vw_rx_start();                 // Start the receiver PLL running

  // Initialise the 7-segment
  pinMode(SEG_1_LE_pin, OUTPUT);
  pinMode(SEG_2_LE_pin, OUTPUT);
  pinMode(SEG_3_LE_pin, OUTPUT);
  pinMode(SEG_4_LE_pin, OUTPUT);
  digitalWrite(SEG_1_LE_pin, HIGH);
  digitalWrite(SEG_2_LE_pin, HIGH);
  digitalWrite(SEG_3_LE_pin, HIGH);
  digitalWrite(SEG_4_LE_pin, HIGH);
  pinMode(SEG_DATA1, OUTPUT);
  pinMode(SEG_DATA2, OUTPUT);
  pinMode(SEG_DATA3, OUTPUT);
  pinMode(SEG_DATA4, OUTPUT);

  pinMode(led_pin, OUTPUT);

  displayMode = 1; /* 1=Clock, 2=counter running */
  dayTime[0] = 55;
  dayTime[1] = 14;

  countDownTick = 6;

}
unsigned long oldTime;

void loop()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  uint8_t tmp_timer;
  unsigned char loopTimer;
  unsigned long masterClock, slaveClock;
  long clockDiff;
  


//  digitalWrite(led_pin, HIGH); // Flash a light to show received good message
//  Message with a good checksum received, print it.

//  digitalWrite(led_pin, LOW);

  if (millis() - oldTime > 100){ /* 100 ms task */
    oldTime = oldTime + 100;  
    ClockTick();
  }

  loopTimer = clockTimer8;
  if (loopTimer > 59){
    clockTimer8 = clockTimer8 - 60;
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

  /* buf[0] = Mode
   *            - Bxxxxxxx0 = Clock
   *            - Bxxxxxxx1 = Timer
   *            - Bxxxxxxxx = 
   * buf[1] = Current time /h (Clock mode) Counter start time /h (Timer mode)
   * buf[2] = Current time /m (Clock mode) Counter start time /m (Timer mode)
   * buf[3] = Current time /s (Clock mode) Counter start time /s (Timer mode)
   * buf[4] = Current time /100ms (Clock mode) Counter start time /100ms (Timer mode)
   */
  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    int i;
    digitalWrite(led_pin, HIGH); // Flash a light to show received good message

    for (i = 0; i < buflen; i++)
    {
      Serial.print(buf[i], DEC);
      Serial.print(' ');
    }

    masterClock = buf[1] * 36000;
    masterClock = masterClock + (unsigned (buf[2]) * 600);
    masterClock = masterClock + (unsigned (buf[3]) * 10);
    masterClock = masterClock + buf[4];
    
    slaveClock = dayTime[1] * 36000;
    slaveClock = slaveClock + (unsigned (dayTime[0]) * 600);
    slaveClock = slaveClock + (unsigned (loopTimer) * 10);
    slaveClock = slaveClock + isrTick8;
    
    Serial.print(masterClock, DEC);
    Serial.println();
    digitalWrite(led_pin, LOW);


    Serial.print(displayMode, DEC);
    Serial.print(' ');
    Serial.print(dayTime[1], DEC);
    Serial.print(' ');
    Serial.print(dayTime[0], DEC);
    Serial.print(' ');
    Serial.print(loopTimer, DEC);
    Serial.print(' ');
    Serial.print(isrTick8, DEC);
    Serial.print(' ');
    Serial.print(slaveClock, DEC);
    Serial.print(' ');
    clockDiff = masterClock - slaveClock;
    Serial.print(clockDiff, DEC);
    Serial.print(' ');
    Serial.print(clockJitter, DEC);

    Serial.println();
    Serial.println();


    if ((clockDiff > 256) or (clockDiff < -256)){
      dayTime[1] = buf[1];
      dayTime[0] = buf[2];
      clockTimer8 = buf[3];
      isrTick8 = buf[4];
      clockJitter = 0;
    } else clockJitter = clockDiff;
  }
  if (isrTick8add == 0) {
    if (clockJitter > 0){
      isrTick8add = 1;
      clockJitter--;
    }
    if (clockJitter < 0){
      isrTick8add = -1;
      clockJitter++;
    }
  }
//  if (countDownTick == 0) displayMode = 1;
  if (displayMode > 0) displayWrite(dayTime);
}


void ClockTick(void){
  char i;
  isrTick8++;
  isrCountDownTick8--;

  if(isrTick8 == 5) {
      isrTick8 = signed (isrTick8) + isrTick8add;  /* handle jitter */
      isrTick8add = 0;
   }
  

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
  PORTB = segment1;
  digitalWrite(SEG_1_LE_pin, LOW);
  digitalWrite(SEG_1_LE_pin, HIGH);
  PORTB = segment2;
  digitalWrite(SEG_2_LE_pin, LOW);
  digitalWrite(SEG_2_LE_pin, HIGH);
  PORTB = segment3;
  digitalWrite(SEG_3_LE_pin, LOW);
  digitalWrite(SEG_3_LE_pin, HIGH);
  PORTB = segment4;
  digitalWrite(SEG_4_LE_pin, LOW);
  digitalWrite(SEG_4_LE_pin, HIGH);    
}
void displayWrite(uint8_t *input){
  uint8_t temp;
  temp = input[1] / 10;
  segment1 = temp;
  segment2 = input[1] - temp*10;
  temp = input[0] / 10;
  segment3 = temp;
  segment4 = input[0] - temp*10;
}

