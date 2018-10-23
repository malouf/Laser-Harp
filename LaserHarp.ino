#include <TimerOne.h>

int beep = 9;
int tilt=13; // Tilt switch pin
int laserPin = 12; // The transistor that controls the laser on and off is connected to the D12 pin
int sensi = 950; // Set the sensitivity of the laser string(tone) R1 pin
const unsigned short int sampleRate = 20000;
char totalValue = 0;
const int buttonsPin[3] =
{
  2,
  3,
  4  
};

const int lasersPin[7] =
{
  A0,
  A1,
  A2,
  A3,
  A4,
  A5,
  7
};

const unsigned short int periods[7] =
{
  sampleRate / 380,
  sampleRate / 445,
  sampleRate / 505,
  sampleRate / 590,
  sampleRate / 668,
  sampleRate / 745,
  sampleRate / 880
};

struct SquareGenerator
{
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
  void counterInc()
  {
    counter++;
  }
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

struct PwmGenerator
{
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
  void counterInc()
  {
    counter++;
  }
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
char tilted;
char buttonsPressed[3];
char buttonsHold[3];
typedef enum { PWM, SQUARE, STACCATO, ARPEGGIATOR } Modes;
Modes mode;

void setup()
{
  for (int i = 0; i < 7; ++i)
  {
    squareGenerator[i].init(periods[i]);
    pwmGenerator[i].init(periods[i]);
  }
  for (int i = 0; i < 3; ++i)
    pinMode(buttonsPin[i], INPUT);
  mode = PWM;
  Timer1.initialize(50); // initialize timer1 10KHz
  Timer1.pwm(beep, 512); // setup pwm on pin 9
  Timer1.attachInterrupt(timerIsrPwm); // attaches timerIsr() as a timer overflow interrupt
  pinMode(7, INPUT); // Set the D7 pin to input mode
  pinMode(laserPin, OUTPUT); // Set the D12 pin to output mode
  pinMode(tilt,INPUT);//Set D13 pin to input mode  
  digitalWrite(laserPin, HIGH); // Turn on the lasers !
  delay(100);
  Serial.begin(9600);
}

void readInput()
{
  char newState;
  // Lasers state
  for (int i = 0; i < 6; ++i)
  {
    newState = analogRead(lasersPin[i]) < sensi;
    lasersPressed[i] = newState && !lasersHold[i];
    lasersHold[i] = newState;
  }
  // Since pin7 digital...
  newState = digitalRead(lasersPin[6]);
  lasersPressed[6] = newState && !lasersHold[6];
  lasersHold[6] = newState;

  // Buttons state
  for (int i = 0; i < 3; ++i)
  {
    newState = digitalRead(buttonsPin[i]);
    buttonsPressed[i] = newState && !buttonsHold[i];
    buttonsHold[i] = newState;
  }

  // Tilt state
  tilted = digitalRead(tilt);

}

void changeMode()
{
  if (buttonsPressed[0])
  {
    switch(mode) {
      case PWM :
        mode = SQUARE;
        Timer1.attachInterrupt(timerIsrSquare);        
        break;
      case SQUARE :
        mode = STACCATO;
        Timer1.attachInterrupt(timerIsrSquare);
        break;
      case STACCATO :
        mode = PWM;
        Timer1.attachInterrupt(timerIsrPwm);        
        break;
      default :
        break;
    }
  }
}

void loopSquare()
{
  for (int i = 0; i < 7; ++i)
  {
    if ( (lasersHold[i] && mode == SQUARE) ||
         (lasersPressed[i] && mode == STACCATO) )
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
  changeMode();
  switch(mode) {
    case PWM :
      loopPwm();
      break;
    case SQUARE :
    case STACCATO :
      loopSquare();
      break;
    default :
    break;
  }
  delay(15);
}

// --------------------------
// Custom ISR Timer Routine
// --------------------------
void timerIsrSquare()
{
  Timer1.setPwmDuty(beep,(((int) totalValue) + 128) << 2);
  totalValue = 0;
  for (int i = 0; i < 7; ++i)
  {
    squareGenerator[i].counterInc();
    totalValue += squareGenerator[i].getValue();
  }
}

void timerIsrPwm()
{
  Timer1.setPwmDuty(beep,(((int) totalValue) + 128) << 2);
  totalValue = 0;
  for (int i = 0; i < 7; ++i)
  {
    pwmGenerator[i].counterInc();
    totalValue += pwmGenerator[i].getValue();
  }
}
