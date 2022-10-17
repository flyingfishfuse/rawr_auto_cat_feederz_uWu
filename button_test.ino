/******************************************************************************
Pin Description
******************************************************************************/
/*
For pin description of Arduino UNO, let us assume some basic numbering. Let
the numbering begin with the RX Pin (D0). 
	So, RX is Pin 1, 
		TX is Pin 2, 
		D2 is Pin 3 
		and so on.

On the other side, NC is Pin 19, IOREF is Pin 20 etc. 
Overall, there are 32 pins on the Arduino UNO Board.

IN CODE, the pinds use the "digital" number, 
	i.e 
		physical pin3 is digital pin 2, and you put THAT number in the code

on my arduino UNO, the board is labled with the digital pin names
and not the physical. So you use the pin numbers on the board in the code

Pin Number			Pin Name			Description		Alternative Functions

1 RX/D0		|	Digital IO Pin 0	|	Serial RX Pin Generally used as RX
2 TX/D1		|	Digital IO Pin 1	|	Serial TX Pin Generally used as TX
3 D2		|	Digital IO Pin 2	|	PWM
4 D3		|	Digital IO Pin 3	|	Timer (OC2B)
5 D4		|	Digital IO Pin 4	|	Timer (T0/XCK), PWM
6 D5		|	Digital IO Pin 5	|	Timer (OC0B/T1), PWM
7 D6		|	Digital IO Pin 6	|
8 D7		|	Digital IO Pin 7	|
9 D8		|	Digital IO Pin 8	|	Timer (CLK0/ICP1), PWM
10 D9		|	Digital IO Pin 9	|	Timer (OC1A), PWM
11 D10		|	Digital IO Pin 10	|	Timer (OC1B), PWM
12 D11		|	Digital IO Pin 11	|	SPI (MOSI) Timer (OC2A)
13 D12		|	Digital IO Pin 12	|	SPI (MISO)
14 D13		|	Digital IO Pin 13	|	SPI (SCK)
15 GND		|	Ground				|	
16 AREF		|	Analog Reference	|
17 SDA/D18	|	Digital IO Pin 18	|	I2C Data Pin
18 SCL/D19	|	Digital IO Pin 19	|	I2C Clock Pin
19 NC		|	Not Connected	
20 IOREF	|	Voltage Reference	
21 RESET	|	Reset (Active LOW)	
22 3V3		|	Power
23 5V		|	+5V Output from regulator or +5V regulated Input	
24 GND		|	Ground
25 GND		|	Ground	
26 VIN		|	Unregulated Supply
27 A0		|	Analog Input 0	Digital IO Pin 14
28 A1		|	Analog Input 1	Digital IO Pin 15
29 A2		|	Analog Input 2	Digital IO Pin 16
30 A3		|	Analog Input 3	Digital IO Pin 17
31 A4		|	Analog Input 4	Digital IO Pin 18 I2C (SDA)
32 A5		|	Analog Input 5	Digital IO Pin 19 I2C (SCL)

The following table describes the pins of the ICSP Connector.

MISO	Master In Slave Out (Input or Output)
5V		Supply
SCK		Clock (from Master to Slave)
MOSI	Master Out Slave In (Input or Output)
RESET 	Reset (Active LOW) 
GND		Ground

*/
/******************************************************************************
Imports
******************************************************************************/
#include <SoftwareSerial.h>
#include <Arduino.h>


/******************************************************************************
PINOUT
******************************************************************************/
// Button pin, for opening and closing the food door
//Digital IO Pin 8
#define FLAPOPENBUTTONPIN 4

// add cat to registry button, adds an rfid ID number to eeprom
//Digital IO Pin 7
#define ADDTOCATREGISTRYPIN 5

//button pin, to clear registry data, resets stored rfid data to 0's
//Digital IO Pin 6	
#define CLEARCATREGISTRYPIN 6

/******************************************************************************
Button Variables
******************************************************************************/
int add_to_registry;
int clear_registry;
int actuate_door_button;

/******************************************************************************
SETUP 
******************************************************************************/
void setup() {
	// setup RFID
	// Attach buttons to inputs
	pinMode(ADDTOCATREGISTRYPIN, INPUT);
	pinMode(CLEARCATREGISTRYPIN, INPUT);
	pinMode(FLAPOPENBUTTONPIN, INPUT);

    Serial.begin(9600); 

    Serial.println("[+] INIT DONE"); 
}

/******************************************************************************
read button states
******************************************************************************/
void read_buttons(){
	// TODO: do something stupid if all buttons pressed,
	// make led flash funny?
	add_to_registry = digitalRead(ADDTOCATREGISTRYPIN);
	clear_registry = digitalRead(CLEARCATREGISTRYPIN);
	actuate_door_button = digitalRead(FLAPOPENBUTTONPIN);
}

/******************************************************************************
main loop
******************************************************************************/
void loop() {

	// read button states to determine control flow
	read_buttons();


	/*************************
	BUTTON ACTIONS
	**************************/
	// if button to clear registry is pressed
	if (clear_registry){
		Serial.println("Clear registry button pressed");
	}
	
	// if button to register cat is pressed
	if (add_to_registry == 1);{
		Serial.println("Add to registry button pressed");
	}
	// if button to change door position is pressed
	if (actuate_door_button == 1){
		Serial.println("Acutate door button pressed");
	}


}	

