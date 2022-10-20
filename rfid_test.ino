/*
RFID reading code
*/

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
#include <rdm6300.h>
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

/******************************************************************************
RFID variables
******************************************************************************/
Rdm6300 rdm6300;

// testing a modification, remove if necessary
// this is to try and prevent too many EEPROM reads
// stores tag from eeprom in memory on boot
uint32_t registered_tag = 0xdeadbeef;

// trigger switch, is true when rfid tag is detected in device
bool rfid_detected;

// used to store an incoming data frame 
uint32_t rfid_tag_in_device;

// is allowed to use?
bool is_allowed = false;

bool tag_still_there = false;

/******************************************************************************
Node to perform functions
Connect this to other code
******************************************************************************/
void do_something(){
    Serial.println("[+] DOING SOMETHING IMPORTANT!")
}
/******************************************************************************
Check if cat in scanner is allowed to access feeder
Compares tag in device with tag stored in EEPROM

MOVED: sets tag data into global stored_rfid_tag
		moved to setup()
******************************************************************************/
bool check_tag(uint32_t tag_to_check){
	Serial.println("Reading tag in rfid field");

	// stored tag data is already loaded into memory from eeprom
	// compare with tag being read in device
	if (tag_in_device == registered_tag) {
		Serial.println("[+] Stored tag matches tag in device");
		return true;
	}
	if (rfid_tag_in_device != registered_tag){
		Serial.println("[-] Stored tag does not match tag in device");
		return false;
	}
}

/******************************************************************************
RFID field observation code
	Checks for tag in field area
	if tag in field, stores tag data for later validation
	Sets rfid_detected to true
******************************************************************************/
void check_rfid(){

	// returns 0 if no tag near, or in second itteration
    //rfid_tag_in_device = rdm6300.get_new_tag_id()

	//get tag information from cat near device, returns uint32_t
	rfid_tag_in_device = rdm6300.get_tag_id();
	// set trigger if device near
    if (rfid_tag_in_device != 0) {
		Serial.println("RFID TAG INFO:");
    	Serial.println(rfid_tag_in_device,HEX);      
		rfid_detected = true;
    }
	// remove trigger if tag not in field
	// if another cat is there, tag will not match check_cat()
	if (rfid_tag_in_device = 0) {
        Serial.println("[-] No RFID tag in device detection field")
		rfid_detected = false;
	}
}
/******************************************************************************
RFID behaviors, actions to undertake when tag in field
******************************************************************************/
void rfid_loop(){
		// checks if cat is allowed to use feeder
		is_allowed = check_tag(rfid_tag_in_device);
		// cat IS allowed to use feeder
		if (is_allowed){
			do_something();
			// set trigger to let device know cat is present
			cat_still_there = true;
			// wait until the cats rfid tag is no longer in the detection field
			//wait_for_cat_to_finish();
		}
		// this previously was a seperate function
		//void wait_for_cat_to_finish()
		while (cat_still_there){
		// check if tag still present
			check_rfid();

			// if it isnt, close the door
			// and set trigger to let device know cat is not there
			if (rfid_detected == false){
				//cat_still_there = false;
				close_door();
				break;
			}

			// tag still in rfid field
			if (rfid_detected) {
				// wait a second and check again
				delay(1000);
			}
		}
}
/******************************************************************************
SETUP 
******************************************************************************/
void setup() {
	// attach servo pin
	food_flap.attach(CAT_FLAP_SERVO_PIN);
    Serial.begin(9600); 
    rdm6300.begin(RFID_RX_PIN);
    Serial.println("[+] INIT DONE");

	// the chances of a cat having the rfid 
	// data match whats there from the factory is super low.
	// you MUST set a cat tag using button 2 before the door will open
	registered_tag = uint32_t 2334455566789
	
}
/******************************************************************************
main loop
******************************************************************************/
void loop() {
	/*************************
	SCAN FOR NEARBY RFID TAGS
	*************************/
	check_rfid();

	/***************************
	RFID DETECTION AND BEHAVIOR
	***************************/
    // tag is near device
	if (rfid_detected) {
		rfid_actions();
		}
		// rfid tag not in detection field
		else{
            // wait, prevents cluttering of serial monitor stream
			delay(3000)
		}			
}	

