/******************************************************************************
This is the code for the automatic cat litter cylinder I am making.
It uses 3-d printed gears and fiber composite cylinder to roll teh poo away!

I *think* all the parts should be readily available on amazon or aliexpress
for a reasonable price. I can probably build it all for you and shit... ship it
for home assembly for a reasonable price but it's probably best if you do it.

How else will you learn enough to fix it if a part dies?

// PARTS LIST
/*
ARDUINO BOARD
2 potentiometers (timing of rollers)
A SPST push button (begin rolling)
Another SPST push button (emergency stop)

a 3d printer!


epoxy and fabric (make a giant tube to hold the cat poo)
hand tools

******************************************************************************/
// PINOUT
// L_EN -> 8
// R_EN -> 8
// L_PWM -> 9
// R_PWM -> 10
/******************************************************************************
imports for functionality
******************************************************************************/
#include <Arduino.h>
#include "BTS7960.h"

/******************************************************************************
variable declarations and pin assignments
******************************************************************************/
const uint8_t EN = 8;
const uint8_t L_PWM = 9;
const uint8_t R_PWM = 10;

BTS7960 motorController(EN, L_PWM, R_PWM);

// pins for motor shield
int motor_pin1
int motor_pin2

/******************************************************************************
Potentiometer pinout and variables
******************************************************************************/
//////////////////////////////////
// pot1
int speed_control_pin = A1
// speed_control potentiometer
int speed

//////////////////////////////////
//pot2
int timer_dial_pin1 = A0;
// variable to store the value coming from the sensor
int timerval = 0;

/******************************************************************************
Button pinout and variables
******************************************************************************/
// the pin that the pushbutton is attached to
const int  buttonPin = 2;
// current state of the button
int buttonState = 0;
// counter for the number of button presses
int buttonPushCounter = 0;
// previous state of the button
int lastButtonState = 0;

/******************************************************************************
Read the input device for modifying the delay timing in motor reverwsal/shutoff
******************************************************************************/
void read_timer_input(){
  timerval = analogRead(timer_dial_pin);
}

/******************************************************************************
Read the input for actuating the cylinder
******************************************************************************/
void read_clean_input(){
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);
}

/******************************************************************************
Controls the speed of the motor for precise control of start and stop position
as controlled by the potentiometer or other variable input mechanism
******************************************************************************/
void read_speed_input(){
  speed = analogRead(speed_control_pin)
}
/******************************************************************************
Controls the timing of the motor so it spins for a specified amount of time
as controlled by the potentiometer or other input mechanism

-------!!! IMPORTANT !!!-------
USE THIS IN PLACE OF REGULAR delay() function!!!
-------!!! IMPORTANT !!!-------

******************************************************************************/
void wait_for_motor_to_spin(){
  //read_timer_input()
  delay(timerval * 10);
}

/******************************************************************************
Drives the motor in the forward direction, to separate the sand and poop
The poop will fall into a hole that leads to a poop holder lines with a bag
******************************************************************************/
void motor_forward(){
  motorController.Enable();

  stop_motor()
}

/******************************************************************************
Drives the motor in reverse, rolling the barrel back into primary position
to allow the sand to pour out of the separation chamber into the poop area
******************************************************************************/
void motor_reverse(){
  motorController.Enable();
  motorController.TurnLeft(speed);
  stop_motor()
}

/******************************************************************************
Stops the motor after running
******************************************************************************/
void stop_motor(){
  motorController.Stop();
  motorController.Disable();
}

/******************************************************************************
Run the main feature of the litter box when the button is pressed
******************************************************************************/
void clean_da_poopie(){
  // roll forward
  motor_forward()
  // wait
  wait_for_motor_to_spin()
  // roll back into place
  motor_reverse()
  //stop motor!
  stop_motor()
}
void setup() {
  // initialize the input pins:
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(speed_control_pin, INPUT_PULLUP)
  pinMode(timer_dial_pin, INPUT_PULLUP)

  // enable debugging via serial
  Serial.begin(9600);

}

void loop() {
  // read speed control input
  read_speed_input()
  // read timing control input
  read_timer_input()
  // when button is pressed, clean da poopie
  read_clean_input()

    // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == LOW) {
      // if the current state is LOW then the button went from off to on:
      Serial.println("on");

      // clean the litter! 
      clean_da_poopie()
    } else {
      // if the current state is HIGH then the button went from on to off:
      Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  // save the current state as the last state, for next time through the loop
  lastButtonState = buttonState;


  // basic functions for motor
  //motorController.Enable();
  //motorController.Stop();
  //motorController.Disable();
}
