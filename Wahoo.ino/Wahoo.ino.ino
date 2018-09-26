int redPin = 5;  //Connect the R pin of the RGB LED to the D5 pin
int greenPin = 6;//Connect the G pin of the RGB LED to the D6 pin
int bluePin = 9; //Connect the B pin of the RGB LED to the D9 pin
int beep=10;//Buzzer is connected to D8 Pin
int pin = bluePin;

#include <TimerOne.h>
 
void setup() 
{
  // Initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards
  //pinMode(redPin, OUTPUT);
  //pinMode(pin, OUTPUT);
  //digitalWrite(pin, LOW);
  Timer1.initialize(1000000); // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-and-off, per second)
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here
}
 
void loop()
{
  // Main code loop
  // TODO: Put your regular (non-ISR) logic here
}
 
/// --------------------------
/// Custom ISR Timer Routine
/// --------------------------
void timerIsr()
{
    // Toggle LED
    Timer1.pwm(pin, 30, 1000000);
}
