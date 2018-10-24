#include <TimerOne.h>

int redPin = 5;  //Connect the R pin of the RGB LED to the D5 pin
int greenPin = 6;//Connect the G pin of the RGB LED to the D6 pin
int beep = 9;
int tilt = 13; // Tilt switch pin
int laserPin = 12; // The transistor that controls the laser on and off is connected to the D12 pin
int sensi = 950; // Set the sensitivity of the laser string(tone) R1 pin
const unsigned short int sampleRate = 20000;
char totalValue = 0;
short int filterValue = 512;
char lasersPressed[7];
char lasersHold[7];
char tilted;
char buttonsPressed[3];
char buttonsHold[3];
typedef enum { PWM, SQUARE, STACCATO, ARPEGGIATOR } Modes;

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

const unsigned char periods[2][7] =
{
  {
    // Guqin strings
    sampleRate / 380,
    sampleRate / 445,
    sampleRate / 505,
    sampleRate / 590,
    sampleRate / 668,
    sampleRate / 745,
    sampleRate / 880
  },
  {
    // A Blues scale
    sampleRate / 445,
    sampleRate / 505,
    sampleRate / 590,
    sampleRate / 620,
    sampleRate / 668,
    sampleRate / 745,
    sampleRate / 880
  },
};

const unsigned char scaleCount = sizeof(periods) / sizeof(periods[0]);
unsigned char currentScale = 0;

struct SquareGenerator
{
  unsigned char period;
  char volume;
  char phase;
  char value;
  unsigned char counter;
  void init(const unsigned char newPeriod)
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
  void setPeriod(unsigned char newPeriod)
  {
    period = newPeriod;
  }
  void slidePeriod(unsigned char targetPeriod)
  {
    if ( (targetPeriod >= period - 2) ||
         (targetPeriod <= period + 1) )
      period = targetPeriod; // Stable value: use exact period
    else
      period = period >> 1 + targetPeriod >> 1; // Slide using an approximate 1st order filter
  }
};

struct PwmGenerator
{
  unsigned char periodHigh;
  unsigned char periodLow;
  char volume;
  char phase;
  char sweepPhase;
  char value;
  unsigned char counter;
  const unsigned char periodMin = 5;
  void init(const unsigned char newPeriod)
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
  void setPeriod(unsigned char newPeriod)
  {
    periodHigh = periodMin;
    periodLow = newPeriod - periodMin;
    sweepPhase = 0;
  }
};

struct Arpeggiator
{
  unsigned short int counter;
  unsigned short int tempo;
  unsigned short int lastTonePlayed;
  unsigned short int toneToPlay;
  char tonePlayed[7];
  void resetToneToPlay() { toneToPlay = 10; }
  void resetTonePlayed()
  {
    for (int i = 0; i < 7; ++i)
      tonePlayed[i] = 0;
    lastTonePlayed = 0;
  }
  void findNextToneToPlay()
  {
    // if mode == going up
    for (int i = lastTonePlayed; i < 7; ++i)
    {
      if (lasersHold[i] && !tonePlayed[i])
      {
        toneToPlay = i;
        break;
      }
    }
    for (int i = lastTonePlayed; i < 7; ++i)
    {
      if (lasersHold[i] && !tonePlayed[i])
        break;
      if (i + 1 == 7)
        resetTonePlayed();
    }
  }
  void setTempo(unsigned char newTempo) {tempo = newTempo;}
  void tempoInc()
  {
    tempo++;
    if (tempo >= 20)
      tempo = 7;
  }
  void counterInc() {counter++;}
  void counterReset() {counter = 0;}
  void init()
  {
    counter = 0;
    tempo = 7;
    resetTonePlayed();
    resetToneToPlay();
  }
};

SquareGenerator squareGenerator[7];
PwmGenerator pwmGenerator[7];
Arpeggiator arpeggiator;
Modes mode;

void setup()
{
  for (int i = 0; i < 7; ++i)
  {
    squareGenerator[i].init(periods[currentScale][i]);
    pwmGenerator[i].init(periods[currentScale][i]);
  }
  for (int i = 0; i < 3; ++i)
  {
    pinMode(buttonsPin[i], INPUT);
    buttonsPressed[i] = 0;
    buttonsHold[i] = 0;
  }
  arpeggiator.init();
  mode = PWM;
  colorRG(255, 255); // Blue
  Timer1.initialize(50); // initialize timer1 10KHz
  Timer1.pwm(beep, 512); // setup pwm on pin 9
  Timer1.attachInterrupt(timerIsrPwm); // attaches timerIsr() as a timer overflow interrupt
  pinMode(7, INPUT); // Set the laser D7 pin to input mode
  pinMode(laserPin, OUTPUT); // Set the D12 pin to output mode
  pinMode(tilt,INPUT);//Set D13 pin to input mode 
  pinMode(redPin, OUTPUT);//Set the D5 pin to output mode
  pinMode(greenPin, OUTPUT);//Set the D6 pin to output mode
  digitalWrite(laserPin, HIGH); // Turn on the lasers !
  delay(100);
  Serial.begin(9600);
}

void colorRG(int red, int green){
  analogWrite(redPin,constrain(red,0,255));
  analogWrite(greenPin,constrain(green,0,255));
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
    buttonsPressed[i] = !newState && !buttonsHold[i]; // Buttons when pushed are == 0
    buttonsHold[i] = !newState;
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
        colorRG(0, 255); // Pink
        break;
      case SQUARE :
        mode = STACCATO;
        Timer1.attachInterrupt(timerIsrSquare);
        colorRG(255, 0); // Cyan
        break;
      case STACCATO :
        mode = ARPEGGIATOR;
        Timer1.attachInterrupt(timerIsrSquare);
        arpeggiator.init(); // White
        colorRG(0, 0);         
        break;
      case ARPEGGIATOR :
        mode = PWM;
        Timer1.attachInterrupt(timerIsrPwm);
        colorRG(255, 255); // Blue
        break;
      default :
        break;
    }
  }

  if (buttonsPressed[1])
  {
    currentScale++;
    if (currentScale >= scaleCount)
      currentScale = 0;
    for (int i = 0; i < 7; ++i)
    {
      squareGenerator[i].setPeriod(periods[currentScale][i]);
      pwmGenerator[i].setPeriod(periods[currentScale][i]);
    }
  }

  if (buttonsPressed[2])
  {
    arpeggiator.tempoInc();
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

    if (tilted)
      squareGenerator[i].slidePeriod(periods[currentScale][i]/2); // Slide to octave up when tilted
    else
      squareGenerator[i].slidePeriod(periods[currentScale][i]);
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

void loopArpeggiator()
{
  arpeggiator.counterInc();
  arpeggiator.findNextToneToPlay();
  if (arpeggiator.counter >= arpeggiator.tempo)
  {
    arpeggiator.counterReset();
    if (arpeggiator.toneToPlay < 7)
    {
      arpeggiator.lastTonePlayed = arpeggiator.toneToPlay;
      arpeggiator.tonePlayed[arpeggiator.toneToPlay] = 1;
      squareGenerator[arpeggiator.toneToPlay].volume = 127 / (sizeof(squareGenerator) / sizeof(squareGenerator[0]));
      arpeggiator.resetToneToPlay();
    }
  }
  for (int i = 0; i < 7; ++i)
  {
    if (squareGenerator[i].volume > 0)
      squareGenerator[i].volume--;
    if (tilted)
      squareGenerator[i].slidePeriod(periods[currentScale][i]/2); // Slide to octave up when tilted
    else
      squareGenerator[i].slidePeriod(periods[currentScale][i]);
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
    case ARPEGGIATOR :
      loopArpeggiator();
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
  Timer1.setPwmDuty(beep, filterValue);
  totalValue = 0;
  for (int i = 0; i < 7; ++i)
  {
    squareGenerator[i].counterInc();
    totalValue += squareGenerator[i].getValue();
  }

  // Add a simple 1st order filter to have a less harsh square sound
  filterValue = (filterValue >> 1) + (filterValue >> 2) + (((int) totalValue) + 128);
}

void timerIsrPwm()
{
  Timer1.setPwmDuty(beep, filterValue);
  totalValue = 0;
  for (int i = 0; i < 7; ++i)
  {
    pwmGenerator[i].counterInc();
    totalValue += pwmGenerator[i].getValue();
  }

  // Add a simple 1st order filter to have a less harsh square sound
  filterValue = (filterValue >> 1) + (filterValue >> 2) + (((int) totalValue) + 128);
}
