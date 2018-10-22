# Laser Harp

---
*IMPORTANT*
Using the arduino uno, we can have a buzzer on the pin 9 or 10 for PWM, unfortunately Adeept put it on the pin 8 so we are using a pin bridge on the pin 8 and 9)
This needs to be done in order to make the harp sound !
---

Using the laser harp from [Adeept](http://www.adeept.com/adeept-laser-piano-laser-harp-kit-for-arduino-uno-r3-steam-starter-kit-arduino-starter-kit-with-tutorials_p0107_s0025.html), we wanted to expand the features the harp has.
So the final stable version we're aiming for is :
1. Have the 7 lasers pin playing simulteanous tones.
2. Switch between 3 modes : PWMc (strings), Square wave with delay, Staccato square wave
3. Switch between presetted scales
4. Switch octave up or down
5. Add an arpeggiator mode

# How it's done

We are using the Timer1 library so we can use the PWM duty cycle inside an interuption to generate a wave on the pin 9.

---
*IMPORTANT*
Using the arduino uno, we can have a buzzer on the pin 9 or 10 for PWM, unfortunately Adeept put it on the pin 8 so we are using a pin bridge on the pin 8 and 9)
This needs to be done in order to make the harp sound !
---
