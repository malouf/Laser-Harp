int redPin = 5;  //Connect the R pin of the RGB LED to the D5 pin
int greenPin = 6;//Connect the G pin of the RGB LED to the D6 pin
int bluePin = 9; //Connect the B pin of the RGB LED to the D9 pin
int beep=10;//Buzzer is connected to D8 Pin
int pin = bluePin;

#include <TimerOne.h>

char costable[64] = {
127  ,
126 ,
124 ,
121 ,
117 ,
112 ,
105 ,
98  ,
89  ,
80  ,
70  ,
59  ,
48  ,
36  ,
24  ,
12  ,
0 ,
-13 ,
-25 ,
-37 ,
-49 ,
-60 ,
-71 ,
-81 ,
-90 ,
-99 ,
-106  ,
-113  ,
-118  ,
-122  ,
-125  ,
-127  ,
-127  ,
-127  ,
-125  ,
-122  ,
-118  ,
-113  ,
-106  ,
-99 ,
-90 ,
-81 ,
-71 ,
-60 ,
-49 ,
-37 ,
-25 ,
-13 ,
-1  ,
12  ,
24  ,
36  ,
48  ,
59  ,
70  ,
80  ,
89  ,
98  ,
105 ,
112 ,
117 ,
121 ,
124 ,
126
};

void setup()
{
  pinMode(13, OUTPUT);
  Timer1.initialize(100);              // initialize timer1 10KHz
  Timer1.pwm(pin, 512);               // setup pwm on pin 9
  Timer1.attachInterrupt(timerIsr);   // attaches timerIsr() as a timer overflow interrupt
}
 
void loop()
{
}

unsigned short int counter = 1;
char value = 127;

/// --------------------------
/// Custom ISR Timer Routine
/// --------------------------
void timerIsr()
{
 Timer1.setPwmDuty(pin, (((int)value) + 128) << 2);
 counter++;
 value = costable[(counter * 60 / 20) & 0x3F] >> 1;
 value += costable[(counter * 45 / 20) & 0x3F] >> 1;
}
