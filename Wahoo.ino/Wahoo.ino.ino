#include <TimerOne.h>

int beep = 9;
int pint=A0;//Photoresistor R1 is connected to A0 Pin
int pint2=A1;//Photoresistor R2 is connected to A1 Pin
int pint3=A2;//Photoresistor R3 is connected to A2 Pin
int pint4=A3;//Photoresistor R4 is connected to A3 Pin
int pint5=A4;//Photoresistor R5 is connected to A4 Pin
int pint6=A5;//Photoresistor R6 is connected to A5 Pin
int pint7=7;//Photoresistor R7 is connected to D7 Pin
int val;//Save A0 analog value
int val2;//Save A1 analog value
int val3;//Save A2 analog value
int val4;//Save A3 analog value
int val5;//Save A4 analog value
int val6;//Save A5 analog value
int val7;//Save D7 digital value
int buttonPin = 2; //Button 1 is connected to the D2 pin
int buttonPin1 = 3;//Button 2 is connected to the D3 pin
int buttonPin2 = 4;//Button 3 is connected to the D4 pin
int laserPin=12;//The transistor that controls the laser on and off is connected to the D12 pin
int sensi=950;//Set the sensitivity of the laser string(tone) R1 pin
char totalValue = 0;

struct SquareGenerator {
  SquareGenerator():volume(0), phase(0), value(0){}
  unsigned short int period;
  char volume;
  char phase;
  char value;
  unsigned short int counter;
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

SquareGenerator squareGenerator[2];

void setup()
{
  Timer1.initialize(50);              // initialize timer1 10KHz
  Timer1.pwm(beep, 512);               // setup pwm on pin 9
  Timer1.attachInterrupt(timerIsr);   // attaches timerIsr() as a timer overflow interrupt
  pinMode(pint7,INPUT);//Set the D7 pin to input mode
  pinMode(buttonPin, INPUT);//Set D2 pin to input mode
  pinMode(buttonPin1, INPUT);//Set D3 pin to input mode
  pinMode(buttonPin2, INPUT);//Set D4 pin to input mode
  pinMode(laserPin,OUTPUT);//Set the D12 pin to output mode
  digitalWrite(laserPin,HIGH);// Turn on the lasers !
}

void loop()
{
  val=analogRead(pint);//Read the analog value of the photoresistor R1
  val2=analogRead(pint2);//Read the analog value of the photoresistor R2
  val3=analogRead(pint3);//Read the analog value of the photoresistor R3
  val4=analogRead(pint4);//Read the analog value of the photoresistor R4
  val5=analogRead(pint5);//Read the analog value of the photoresistor R5
  val6=analogRead(pint6);//Read the analog value of the photoresistor R6
  val7=digitalRead(pint7);//Read the digital value of the photoresistor R7

  if (val < sensi)
  {
    squareGenerator[0].period = 20000 / 880;
    squareGenerator[0].volume = 63;
  }
  else 
  {
    if (squareGenerator[0].volume > 0)
    {
      squareGenerator[0].volume--;
    }
  }
  
  if (val2 < sensi)
  {
    squareGenerator[1].period = 20000 / 750;
    squareGenerator[1].volume = 63;
  }
  else 
  {
    if (squareGenerator[1].volume > 0)
    {
      squareGenerator[1].volume--;
    }
  }
  delay(3);
}

/// --------------------------
/// Custom ISR Timer Routine
/// --------------------------
void timerIsr()
{
  Timer1.setPwmDuty(beep, (((int)totalValue) + 128) << 2);
  squareGenerator[0].counterInc();
  squareGenerator[1].counterInc();  
  totalValue = squareGenerator[0].getValue() + squareGenerator[1].getValue();
}
