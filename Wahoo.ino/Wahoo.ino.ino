#include <TimerOne.h>

int beep = 9;
int buttonPin = 2; //Button 1 is connected to the D2 pin
int buttonPin1 = 3;//Button 2 is connected to the D3 pin
int buttonPin2 = 4;//Button 3 is connected to the D4 pin
int laserPin=12;//The transistor that controls the laser on and off is connected to the D12 pin
int sensi=950;//Set the sensitivity of the laser string(tone) R1 pin
const unsigned short int sampleRate = 20000;
char totalValue = 0;
const int lasersPin[7] = {
 A0,
 A1,
 A2,
 A3,
 A4,
 A5,
 7
};

const unsigned short int periods[7] = {
  sampleRate / 380,
  sampleRate / 445,
  sampleRate / 505,
  sampleRate / 590,
  sampleRate / 668,
  sampleRate / 745,
  sampleRate / 880
};

struct SquareGenerator {
  unsigned short int period;
  char volume;
  char phase;
  char value;
  unsigned short int counter;
  void init(const unsigned short int newPeriod)
  {
    volume = 0;
    value = 0;    
    phase = 0;
    period = newPeriod;
    counter = 0;
  }
  void counterInc() {counter++;}
  char getValue()
  {
    if (counter >= period)
    {
      if (phase == 0)
        value = volume;
      else
        value = -volume;
      phase = !phase;
      counter = 0;
    }
    return value;
  }
};

struct PwmGenerator {
  unsigned short int periodHigh;
  unsigned short int periodLow;
  char volume;
  char phase;
  char sweepPhase;
  char value;
  unsigned short int counter;
  const unsigned short int periodMin = 5;
  void init(const unsigned short int newPeriod)
  {
    volume = 0;
    value = 0;    
    phase = 0;
    sweepPhase = 0;
    periodHigh = periodMin;
    periodLow = newPeriod - periodMin;
    counter = 0;
  }
  void pwmSweep()
  {
    if (sweepPhase == 0)
    {
      periodLow--;
      periodHigh++;
      if (periodLow < periodMin)
        sweepPhase = 1;
    }
    else
    {
      periodLow++;
      periodHigh--;
      if (periodHigh < periodMin)
        sweepPhase = 0;
    }
  }
  void counterInc() {counter++;}
  char getValue()
  {
    if (phase == 0)
    {
      if (counter >= periodHigh)
      {
        phase = 1;
        value = volume;
        counter = 0;        
      }
    }
    else
    {
      if (counter >= periodLow)
      {
        phase = 0;
        value = -volume;
        counter = 0;
      }
    }
    return value;
  }
};

SquareGenerator squareGenerator[7];
PwmGenerator pwmGenerator[7];
char lasersPressed[7];
char lasersHold[7];

void setup()
{
  for (int i = 0; i < 7; ++i)
  {
    squareGenerator[i].init(periods[i]);
    pwmGenerator[i].init(periods[i]);
  }  
  Timer1.initialize(50);              // initialize timer1 10KHz
  Timer1.pwm(beep, 512);               // setup pwm on pin 9
  Timer1.attachInterrupt(timerIsrPwm);   // attaches timerIsr() as a timer overflow interrupt
  pinMode(7,INPUT);//Set the D7 pin to input mode
  pinMode(buttonPin, INPUT);//Set D2 pin to input mode
  pinMode(buttonPin1, INPUT);//Set D3 pin to input mode
  pinMode(buttonPin2, INPUT);//Set D4 pin to input mode
  pinMode(laserPin,OUTPUT);//Set the D12 pin to output mode
  digitalWrite(laserPin,HIGH);// Turn on the lasers !
  delay(100);
    Serial.begin(9600);
}

void readInput()
{
  for (int i = 0; i < 6; ++i)
  {
    char newState = analogRead(lasersPin[i]) < sensi;
    lasersPressed[i] = newState && !lasersHold[i];
    lasersHold[i] = newState;
  }
  // Since pin7 digital...
  char newState = digitalRead(lasersPin[6]);
  lasersPressed[6] = newState && !lasersHold[6];
  lasersHold[6] = newState;
}

void loopSquare()
{
  for (int i = 0; i < 7; ++i)
  {
    if (lasersHold[i])
      squareGenerator[i].volume = 127 / (sizeof(squareGenerator) / sizeof(squareGenerator[0]));
    else
      if (squareGenerator[i].volume > 0)
        squareGenerator[i].volume--;
  }
}

void loopPwm()
{
  for (int i = 0; i < 7; ++i)
  {
    pwmGenerator[i].pwmSweep();
    if (lasersHold[i])
    {
      if (pwmGenerator[i].volume < 127 / (sizeof(pwmGenerator) / sizeof(pwmGenerator[0])))
       pwmGenerator[i].volume++;
    }
    else
      if (pwmGenerator[i].volume > 0)
        pwmGenerator[i].volume--;
  }
}

void loop()
{
  readInput();
  loopPwm();
//if button loop square
  delay(15);
}

/// --------------------------
/// Custom ISR Timer Routine
/// --------------------------
void timerIsrSquare()
{
  Timer1.setPwmDuty(beep, (((int)totalValue) + 128) << 2);
  totalValue = 0;
  for (int i = 0; i < 7; ++i)
  {
    squareGenerator[i].counterInc();
    totalValue += squareGenerator[i].getValue();
  }
}

void timerIsrPwm()
{
  Timer1.setPwmDuty(beep, (((int)totalValue) + 128) << 2);
  totalValue = 0;
  for (int i = 0; i < 7; ++i)
  {
    pwmGenerator[i].counterInc();
    totalValue += pwmGenerator[i].getValue();
  }
}
