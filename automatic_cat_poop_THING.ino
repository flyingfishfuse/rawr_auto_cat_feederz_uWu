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

// 12 - clean machine button
// 11 - emergency stop/continue
/******************************************************************************
imports for functionality
******************************************************************************/
#include <Arduino.h>
#include "BTS7960.h"

/******************************************************************************
variable declarations and pin assignments
******************************************************************************/

// MOTOR CONTROL PINS
const uint8_t EN = 8;
const uint8_t L_PWM = 9;
const uint8_t R_PWM = 10;

BTS7960 motorController(EN, L_PWM, R_PWM);

// pins for motor shield
int motor_pin1 ;
int motor_pin2 ;

/*
Motor timing
This is for running the motor after specified periods of time without having
to use the delay() function, thereby preventing you from catching button
presses like altering speed/timing control, and emergency stop

Generally, you should use "unsigned long" for variables that hold time
The value will quickly become too large for an int to store
*/
unsigned long PeviousMotorActivation = 0;  // will store last time motor was activated
long SpinInterval = 30000000;  // 8.3333333 hours

// value for motor speed
int SpeedValue = 100;

int ValueToSetWhenSetButtonPressed = SpeedValue;
// this is necessary because we need to allow for different motors and drums
// as well as different drive trains which all determine the time nececssary to
// complete the full spin cycle of the motor and drum to fully clean and replace
// the sand and remove poop
// value for length of time spinning
// DEFAULT is 250 seconds
int SpinTime = 150000;

/******************************************************************************
Rotary Encoder Inputs and variables
******************************************************************************/

// primary output pulse used to determine the amount of rotation. 
// Each time the knob is turned in either direction by just one detent (click), 
// the ‘CLK’ output goes through one cycle of going HIGH and then LOW.
#define CLK 2

//similar to CLK output, but it lags behind CLK by a 90° phase shift. 
// This output is used to determine the direction of rotation.
#define DT 3

// output of the push button switch (active low). 
// When the knob is depressed, the voltage goes LOW.
#define rotary_function_pin 4

int EncoderClickCount = 0;
int currentStateCLK;
int lastStateCLK;
String EncoderDirection ="";
unsigned long lastButtonPress = 0;
#define rotary_function_pin rotary_function_pin;
int EncoderButtonState;
String EncoderCurrentFunction = "";
// 1 = speed control
// 0 = timing control
// defaults to speed control so we can quickly slow the motor down
int EncoderState = 1;

//////////////////////////////////
//pot2
//int timer_dial_pin1 = A0;
// variable to store the value coming from the sensor
//int timerval = 0;
// DEPRECATED, MOVING TO ROTARY ENCODER WITH THIRD BUTTON TO SWAP FUNCTION
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
Button pinout and variables
******************************************************************************/
// the pin that the pushbutton is attached to
const int  cleanbuttonPin = 12;
// current state of the button
int cleanbuttonState = 0;
// EncoderClickCount for the number of button presses
int cleanbuttonPushEncoderClickCount = 0;
// previous state of the button
int cleanlastButtonState = 0;

int emergencyStopContinue = 11;
// when pressed, will enable use of rotary encoder
int useEncoder = 13;
// when pressed, will change function of encoder
//int RotaryButton = 

/******************************************************************************
Read the input device for modifying the delay timing in motor reverwsal/shutoff
******************************************************************************/
void read_rotary_input(){
  	// Read the current state of CLK
	currentStateCLK = digitalRead(CLK);

	// If last and current state of CLK are different, then pulse occurred
	// React to only 1 state change to avoid double count
	if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

		// If the DT state is different than the CLK state then
		// the encoder is rotating CCW so decrement
		if (digitalRead(DT) != currentStateCLK) {
			EncoderClickCount --;
			EncoderDirection ="CCW";
      // decrease the value of the encoder function
      ValueToSetWhenSetButtonPressed = ValueToSetWhenSetButtonPressed - EncoderClickCount;
		} else {
			// Encoder is rotating CW so increment
			EncoderClickCount ++;
			EncoderDirection ="CW";
      // increase the value of the encoder function
      ValueToSetWhenSetButtonPressed = ValueToSetWhenSetButtonPressed + EncoderClickCount;

		}

		Serial.print("[+] Direction: ");
		Serial.println(EncoderDirection);
		Serial.print("[+] EncoderClickCount: ");
		Serial.println(EncoderClickCount);
    Serial.print("[+] New Value for speed/timer function")
    Serial.println(ValueToSetWhenSetButtonPressed)

	}
  	// Remember last CLK state
	lastStateCLK = currentStateCLK;
}

/*
This function changes the function of the rotary knob to
control either speed of rotation, or timing of unit activation
*/
void ChangeEncoderFunction(){
		//if 50ms have passed since last LOW pulse, it means that the
		//button has been pressed, released and pressed again
		if (millis() - lastButtonPress > 50) {
			Serial.println("Encoder Button pressed! \n Changing function of encoder!");
      if (EncoderState == 1){
        EncoderState = 0;
        EncoderCurrentFunction = "Timing";
        ValueToSetWhenSetButtonPressed = SpinTime;
        Serial.println("Encoder Function Changed To TIMING");
      }
      else if (EncoderState == 0){
        EncoderState = 1;
        EncoderCurrentFunction = "Speed";
        ValueToSetWhenSetButtonPressed = SpeedValue;
        Serial.println("Encoder Function Changed To SPEED");
      }
		}
		// Remember last button press event
		lastButtonPress = millis();

	// Put in a slight delay to help debounce the reading
	delay(1000);

}
/******************************************************************************
Controls the speed of the motor for precise control of start and stop position
as controlled by the potentiometer or other variable input mechanism
******************************************************************************/
/*
void read_encoder_input(){
  speed = analogRead(speed_control_pin)
  
	Serial.print("Speed Direction: ");
	Serial.print(EncoderDirection);
	Serial.print(" | EncoderClickCount: ");
	Serial.println(EncoderClickCount);
}
*/
/******************************************************************************
Controls the spin time of the motor for precise control of start and stop 
position
******************************************************************************/
//void change_spin_time(){

//}
/******************************************************************************
Read the input for actuating the cylinder
******************************************************************************/
void read_clean_input(){
  // read the pushbutton input pin:
  cleanbuttonState = digitalRead(cleanbuttonPin);
      // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the EncoderClickCount
    if (buttonState == LOW) {
      // if the current state is LOW then the button went from off to on:
      Serial.println("on");

      // clean the litter! 
      clean_da_poopie();
    } else {
      // if the current state is HIGH then the button went from on to off:
      Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  // save the current state as the last state, for next time through the loop
  lastButtonState = buttonState;
  delay(1000);
}

/******************************************************************************
Controls the timing of the motor so it spins for a specified amount of time
as controlled by the encoder knob

-------!!! IMPORTANT !!!-------
USE THIS IN PLACE OF REGULAR delay() function!!!
-------!!! IMPORTANT !!!-------

******************************************************************************/
void wait_for_motor_to_spin(){
  // millies() holds a number of milliseconds since the board was powered on
  unsigned long CurrentMillis = millis();
  // if the current time is equal to or greater than the time interval then it must spin
  if (CurrentMillis - SpinTime >= SpinInterval) {
    // save the last time you removed poopies
     = currentMillis;
  }
}

/******************************************************************************
Drives the motor in the forward direction, to separate the sand and poop
The poop will fall into a hole that leads to a poop holder lines with a bag
******************************************************************************/
void motor_forward(){
  Serial.println("Starting motor: FORWARD")
  motorController.Enable();

  stop_motor();
}

/******************************************************************************
Drives the motor in reverse, rolling the barrel back into primary position
to allow the sand to pour out of the separation chamber into the poop area
******************************************************************************/
void motor_reverse(){
  Serial.println("Starting motor: BACKWARD");
  motorController.Enable();
  motorController.TurnLeft(speed);
  stop_motor();
}

/******************************************************************************
Stops the motor after running
******************************************************************************/
void stop_motor(){
  Serial.println("Stopping motor:");
  motorController.Stop();
  motorController.Disable();
}

/******************************************************************************
Run the main feature of the litter box when the button is pressed
******************************************************************************/
void clean_da_poopie(){
  // roll forward;
  motor_forward();
  // wait
  wait_for_motor_to_spin();
  // roll back into place
  motor_reverse();
  //stop motor!
  stop_motor();
}
void setup() {
  // initialize the input pins:
  // clean action
  pinMode(cleanbuttonPin, INPUT_PULLUP);
  // emergency stop / continue
  pinMode(emergencyStopContinue, INPUT_PULLUP);
  // when pressed, activates usage of encoder pin
  //pinMode(useEncoder, INPUT_PULLUP);

  // rotary encoder
 	pinMode(CLK,INPUT);
	pinMode(DT,INPUT);
	// change function pin, changes from speed control to timing control
  pinMode(rotary_function_pin, INPUT_PULLUP);
  
  // enable debugging via serial
  Serial.begin(9600);

  // Read the initial state of CLK
	lastStateCLK = digitalRead(CLK);

}

void loop() {
///////////////////////////////////////////////////////////////////////////////
  // Read the encoder button state
  EncoderButtonState = digitalRead(rotary_function_pin);
  //If we detect LOW signal, button is pressed
	if (EncoderButtonState == LOW) {
    // toggle function of rotary encoder to control either speed or timing
    ChangeEncoderFunction();
  }

  // now we read rotary control input to dial in proper speed and timing
  read_rotary_input();

///////////////////////////////////////////////////////////////////////////////
// MANUAL CLEANING
  // is this button is pressed, the user wants to clean da poopie without waiting
  read_clean_input();

///////////////////////////////////////////////////////////////////////////////
// AUTOMATIC CLEANING ACTUATION BASED ON TIMING

  // check to see if it's time to ROTATE THE POO DRUM; that is, if the difference
  // between the current time and last time you ROTATED THE POO DRUM is bigger than
  // the interval at which you want to ROTATE THE POO DRUM.

  // millies() holds a number of milliseconds since the board was powered on
  unsigned long CurrentMillis = millis();
  // if the current time is equal to or greater than the time interval then it must spin
  if (currentMillis - PeviousMotorActivation >= SpinInterval) {
    // save the last time you removed poopies
    PeviousMotorActivation = currentMillis;
    clean_da_poopie();
  }
  // basic functions for motor
  //motorController.Enable();
  //motorController.Stop();
  //motorController.Disable();
}
