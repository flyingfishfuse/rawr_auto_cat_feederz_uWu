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
#include <Servo.h>
#include <EEPROM.h>
#include <rdm6300.h>
#include <SoftwareSerial.h>
#include <Arduino.h>

/******************************************************************************
Internal functionality
******************************************************************************/
// change for debugging output, button is internal
bool DEBUG = true;

// switch to check if its being run for the first time
bool first_run = true;

/******************************************************************************
servo variables
******************************************************************************/
// create servo object to control a servo
Servo food_flap;

// variable to store the current servo position
int current_position = 0;

// degrees, how far to swing to fully open door
int full_up_position = 45;

// degrees, how far to swing to fully close door
int full_down_position = 0;

//if door is open currently
bool DOOROPEN = false;

/******************************************************************************
PINOUT
******************************************************************************/
//	status: In flux
//	
//	pins chosen for uno and nano

// RX (receive) pin for RFID, plug the TX wire into this, 
// to *receive* the *transmitted* information
//Digital IO Pin 0 Serial RX Pin Generally used as RX
#define RFID_RX_PIN 0


// TX (transmit) pin for RFID, plug the RX wire from the RFID into this 
// to transmit information to the rfid module
//Digital IO Pin 1 Serial TX Pin Generally used as TX
#define RFID_TX_PIN 1

// servo PWM pin, for controlling the food door
//Digital IO Pin 4 PWM
#define CAT_FLAP_SERVO_PIN 3

// Button pin, for opening and closing the food door
//Digital IO Pin 8
#define FLAPOPENBUTTONPIN 4

// add cat to registry button, adds an rfid ID number to eeprom
//Digital IO Pin 7
#define ADDTOCATREGISTRYPIN 5

//button pin, to clear registry data, resets stored rfid data to 0's
//Digital IO Pin 6	
#define CLEARCATREGISTRYPIN 6


// Button pin, this button will be inside the device, enables debugging
// information to print to serial output and changes behavior for testing
//Digital IO Pin 9
//#define DEBUG_BUTTON 9

/******************************************************************************
RFID variables
******************************************************************************/
Rdm6300 rdm6300;

int BUFFER_SIZE = 16;
// trigger switch, is true when rfid tag is detected in device
bool rfid_detected;

// used to store an incoming data frame 
//uint8_t rfid_tag_in_device_u8[BUFFER_SIZE];
uint32_t rfid_tag_in_device;

// rfid tag stored in eeprom
//uint8_t stored_rfid_tag[BUFFER_SIZE];
uint32_t stored_rfid_tag;

// is cat allowed to use the feeder?
bool cat_allowed = false;

bool cat_still_there = false;

/******************************************************************************
EEPROM variables
******************************************************************************/

/*/
Stores cat RFID tag numbers for each cat set to edvice
press button 1 to register a cat
from then on, the door will open for that cat
/*/
#define MAX_CATS_ALLOWABLE 2
#define CATREGISTRYSTARTADDRESS 1
int asdf = sizeof(RDM6300_PACKET_SIZE) * MAX_CATS_ALLOWABLE;
#define REGISTRYSIZE asdf

/******************************************************************************
Button Variables
******************************************************************************/
int add_to_registry;
int clear_registry;
int actuate_door_button;

/******************************************************************************
Opens the door to the cat food

DONE
******************************************************************************/
void open_door(){
	Serial.println("Opening door");
	// from fully closed, to fully open, do something while incrementing by one for every
	// something done
	for (current_position = 0; current_position <= full_up_position; current_position += 1)	{
		// in steps of 1 degree
		// tell servo to go to position
		food_flap.write(current_position);
		// waits 15ms for the servo to reach the position
		delay(15);
	}
}

/******************************************************************************
Closes the door to the cat food

DONE
******************************************************************************/
void close_door(){
		Serial.println("Closing door");
		// from fully open, to fully closed, do something while incrementing by one
		// for every something done
		for (current_position = 0; current_position <= full_up_position; current_position += 1) { 
		// tell servo to go to position in variable 'pos'
		food_flap.write(current_position);
		// waits 15ms for the servo to reach the position
		delay(15);
	}
}

/******************************************************************************
zeros out the eeprom space reserved for the cat registry
******************************************************************************/
void clear_eeprom(){
	Serial.println("Clearing eeprom space");
	for (int index = 0; REGISTRYSIZE; index++){
		EEPROM.write(index, 0);
	}
}
/******************************************************************************
SET a cat's RFID tag to the registry
inputs:
	int      | address
	uint8_t  | byte array with rfid tag number

DONE
******************************************************************************/
//void WriteTagToRegistry(int address, uint8_t tag_data[]){//}, int arraySize) {
void WriteTagToRegistry(int address, uint32_t tag_data){
	//for (int i = 0; i < sizeof(tag_data); i++) {
	//	EEPROM.write(address+i, tag_data[i]);
	//}
	Serial.println("Writing tag to registry");
	EEPROM.put(address, tag_data);
}

/******************************************************************************
READ a cat's RFID tag from the registry
inputs:
	int address      | address to read from
                     |
outputs:             |
                     |
	uint8_t[] GLOBAL | byte array to hold stored data  
******************************************************************************/
//void ReadTagFromRegistry(int address){//, uint8_t rfid_tag_data[], int arraySize) {
void ReadTagFromRegistry(int address){
	Serial.println("Reading tag from registry");
	// read from specified adddress all the way until DATA_TAG_SIZE
	//for (int i = 0; i < sizeof(rfid_tag_in_device); i++) {
	//	// store in global to validate against
	//	stored_rfid_tag[i] = (uint8_t) EEPROM.read(address+i);
	//}
	stored_rfid_tag = (uint32_t) EEPROM.read(address);
}
/******************************************************************************
Setter for cat registry
modify this function to implement more cats allowed

DONE
******************************************************************************/
// uint32_t based code
void set_cat(uint32_t tag_bytes){
	Serial.println("Setting cat in registry");
	WriteTagToRegistry(CATREGISTRYSTARTADDRESS,tag_bytes);
}
/******************************************************************************
Check if cat in scanner is allowed to access feeder
Compares tag in device with tag stored in EEPROM
sets tag data into global stored_rfid_tag
******************************************************************************/
bool check_cat(){
	Serial.println("Checking cat authorization");
	// sets stored_rfid_tag[] with tag number from EEPROM
  	//ReadTagFromRegistry(CATREGISTRYSTARTADDRESS);
	ReadTagFromRegistry(CATREGISTRYSTARTADDRESS);
	// compare with tag being read in device
	if (rfid_tag_in_device == stored_rfid_tag) {
		Serial.println("[+] Stored tag matches tag in device");
		return true;
	}
	if (rfid_tag_in_device != stored_rfid_tag){
		Serial.println("[+] Stored tag matches tag in device");
		return false;
	}
}

/******************************************************************************
Clear the registry of all registered cat RFID tags
******************************************************************************/
void clear_cat_registry(){
	Serial.println("Clearing registry");
	// itterates over entire address space allocated to cat registry
	for (int registry_index = CATREGISTRYSTARTADDRESS ; registry_index < MAX_CATS_ALLOWABLE ; registry_index++){
		EEPROM.write(registry_index, 0);
	}
}

/******************************************************************************
Pauses operation to allow the cat time to eat. Will check if the ID tag is 
in the RFID receiver field , if it is, keeps door open.
Otherwise, it closes the door.

DONE
******************************************************************************/
void wait_for_cat_to_finish(){
	Serial.println("Waiting for cat to finish");
	// TESTING CODE
	if (DEBUG) {
		// wait 5 seconds
	 	delay(5000);
		Serial.println("[+] DEBUG: pretending cat is finished");
	}
	// while the cat is still there
	while (cat_still_there){
		// if cat no longer in field
		if (rdm6300.get_tag_id() == 0){
			//set trigger to let device know cat is not there
			// this kills the loop
			cat_still_there = false;
			//break;
			// wait 2 seconds
			delay(2000);
			close_door();
		}
		else{
			delay(1000);
		}
	}
	// set the trigger to let device know cat is done
	//cat_still_there = false;
}
/******************************************************************************
Operations to perform when turned on for the first time
only use this if you have consistant power to the device. Once the power turns
off, its goign to happen again and you will need to reset the cats
******************************************************************************/
void check_first_run(){
	if (first_run == true) {
		// wait 30 seconds for PIR to normalize
		//Serial.println("[+] Waiting 30 seconds for PIR to normali ze")
		// wait 10 seconds for modules to perform internal operations
		delay(10000);
		// set trigger to off
		first_run = false;
		// 0 out the eeprom to clear old tag info
		//clear_eeprom();
	}
}
/******************************************************************************
SETUP 
******************************************************************************/
void setup() {
	// setup RFID
	// Attach buttons to inputs
	pinMode(ADDTOCATREGISTRYPIN, INPUT);
	pinMode(CLEARCATREGISTRYPIN, INPUT);
	pinMode(FLAPOPENBUTTONPIN, INPUT);

	// attach servo pin
	food_flap.attach(CAT_FLAP_SERVO_PIN);


    Serial.begin(9600); 
    //SerialRFID.begin(9600);
    //SerialRFID.listen(); 

    rdm6300.begin(RFID_RX_PIN);

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
Toggles door open or closed
******************************************************************************/
void actuate_door(){
	Serial.println("Actuating door state");
		// door is open, close door
		if (DOOROPEN == true){
			Serial.println("Door is open");
			close_door();
        	DOOROPEN = false;
		}
		// door is closed, open door
		else if (DOOROPEN == false){
			Serial.println("Door is closed");
			open_door();
        	DOOROPEN = true;
		}
	}
void check_rfid(){
	//get tag information from cat near device, returns uint32_t
	rfid_tag_in_device = rdm6300.get_tag_id();
	// set trigger if device near
    if (rfid_tag_in_device != 0) {
		rfid_detected = true;
    }
}
/******************************************************************************
main loop
******************************************************************************/
void loop() {
    // read rfid
	
	// read button states to determine control flow
	read_buttons();

	/*************************
	SCAN FOR NEARBY RFID TAGS
	*************************/
	check_rfid();

	/*************************
	BUTTON ACTIONS
	**************************/
	// if button to clear registry is pressed
	if (clear_registry){
		clear_cat_registry();
	}
	
	// if button to register cat is pressed
	if (rfid_detected && add_to_registry == 1);{
		set_cat(rfid_tag_in_device);
	}
	// if button to change door position is pressed
	if (actuate_door_button == 1){
		actuate_door();
	}

	/***************************
	RFID DETECTION AND BEHAVIOR
	***************************/
    // returns 0 if no tag near, or in second itteration
    //rfid_tag_in_device = rdm6300.get_new_tag_id()
    // tag is near device
	if (rfid_detected) {
        // print data for debugging
        Serial.println("RFID TAG INFO:");
        // gets tag currently near device, regardless if run second time
	    Serial.println(rfid_tag_in_device, HEX);
		// checks if cat is allowed to use feeder
		cat_allowed = check_cat();
		// cat IS allowed to use feeder
		if (cat_allowed){
			// open the door
			open_door();
			// set trigger to let device know cat is present
			// cant let it get pinched!
			cat_still_there = true;
			// wait until the cats rfid tag is no longer in the detection field
			// of the rfid reader
			// this function also closes the door
			wait_for_cat_to_finish();
		}
		// cat IS NOT allowed to use feeder
		// fuck off asshole
		else{
			//continue;
		}		
	}		
}	

