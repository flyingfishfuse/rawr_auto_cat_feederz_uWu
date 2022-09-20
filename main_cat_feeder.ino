#include <Servo.h>
#include <EEPROM.h>
#include <rdm6300.h>
#include <SoftwareSerial.h>

/*
Open Food Flap by activating servo when cat gets near opening
*/
// create servo object to control a servo
Servo food_flap;
SoftwareSerial SerialRFID = SoftwareSerial(6,8); 	//pins = Pins()

// variable to store the current servo position
int current_position = 0;

// degrees, how far to swing to fully open door
int full_up_position = 45;

// degrees, how far to swing to fully close door
int full_down_position = 0;

//if door is open currently
bool DOOROPEN = false;

/*/
Stores cat RFID tag numbers for each cat set to edvice
press button 1 to register a cat
from then on, the door will open for that cat
/*/
#define MAX_CATS_ALLOWABLE 2
#define CATREGISTRYSTARTADDRESS 1

/*
PINOUT
	status:
		pins chosen for uno and nano
		
*/
#define ADDTOCATREGISTRYPIN 10
#define CLEARCATREGISTRYPIN 9
#define CAT_FLAP_SERVO_PIN 8
#define FLAPOPENBUTTONPIN 7
#define RFID_RX_PIN 6
#define RFID_TX_PIN 5

/*/RFID VARIABLES/*/

// RFID DATA FRAME FORMAT: 1byte head (value: 2), 10byte data (2byte version + 8byte tag), 2byte checksum, 1byte tail (value: 3)
const int BUFFER_SIZE = 14;

// RFID DATA FRAME FORMAT: 1byte head (AA=170), 10byte data (3byte fixed + 2byte country + 5byte tag), 1byte checksum, 1byte tail (BB=187)
//const int BUFFER_SIZE = 13;
// 10byte data (2byte version + 8byte tag)
const int DATA_SIZE = 10;

 // 3 byte fixed (0F 08 00) + 7byte data (2byte country + 5byte tag) (03 84 + 12 DB FA E7 D5)
//const int DATA_SIZE = 10;
 // 3byte fixed (0F 08 00)
const int DATA_FIXED_SIZE = 3;

// 2byte country (example 03 84)
//const int DATA_COUNTRY_SIZE = 2;
// 2byte version (actual meaning of these two bytes may vary)
const int DATA_VERSION_SIZE = 2;

// 8byte tag
const int DATA_TAG_SIZE = 8;

// 5byte tag (example 12 DB FA E7 D5)
//const int DATA_TAG_SIZE = 5;
// 2byte checksum
const int CHECKSUM_SIZE = 2;

// 1byte checksum (example 81)
//const int CHECKSUM_SIZE = 1;

//SoftwareSerial SerialRFID = SoftwareSerial(6,8); 
// trigger switch, is true when rfid tag is detected in rfid loop function
bool rfid_detected;

// used to store an incoming data frame 
uint8_t rfid_tag_in_device[DATA_TAG_SIZE];
int buffer_index = 0;

// actual tag data
uint8_t* TAG_DATA;

// used for reading from EEPROM to compare with tag being read in device
uint8_t rfid_tag_data[DATA_TAG_SIZE];

// is cat allowed to use the feeder?
bool cat_allowed = false;

/******************************************************************************
 checking for a buffer overflow
******************************************************************************/
bool check_buffer_overflow(int buffer_index){
	
		if (buffer_index >= BUFFER_SIZE) { 
			Serial.println("[-] EROOR: Buffer overflow detected!");
			buffer_index = 0;
			return true;
		}
		else{
			return false;
		}
}
/******************************************************************************
Opens the door to the cat food
******************************************************************************/
void open_door(){
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
******************************************************************************/
void close_door(){
		// from fully open, to fully closed, do something while incrementing by one
		// for every something done
		for (current_position = 0; current_position <= full_up_position; current_position += 1) { 
		// tell servo to go to position in variable 'pos'
		food_flap.write(current_position);
		// waits 15ms for the servo to reach the position
		delay(15);
	}
}

////////////////////////////////
// EEPROM FUNCTIONS
////////////////////////////////

/******************************************************************************
SET a cat's RFID tag to the registry
inputs:
	int      | address
	uint8_t  | byte array with rfid tag number
******************************************************************************/
void WriteTagToRegistry(int address, uint8_t tag_data[]){//}, int arraySize) {
	for (int i = 0; i < DATA_TAG_SIZE; i++) {
		EEPROM.write(address+i, tag_data[i]);
	}
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
void ReadTagFromRegistry(int address){//, uint8_t rfid_tag_data[], int arraySize) {
	// read from specified adddress all the way until DATA_TAG_SIZE
	for (int i = 0; i < DATA_TAG_SIZE; i++) {
		// store in global to validate against
		rfid_tag_data[i] = (uint8_t) EEPROM.read(address+i);
	}
}
/******************************************************************************
Setter for cat registry
******************************************************************************/
void set_cat(uint8_t tag_bytes[]){
	WriteTagToRegistry(CATREGISTRYSTARTADDRESS,tag_bytes);
}
/******************************************************************************
Check if cat in scanner is allowed to access feeder
sets tag data into global
******************************************************************************/
bool check_cat(){
	// sets rfid_tag_data[] with stored tag number
  	ReadTagFromRegistry(CATREGISTRYSTARTADDRESS);
	// compare with tag being read in device
  	if (rfid_tag_data == rfid_tag_in_device){
		return true;
	}
	// the tag being read is not in the list of allowed cats
	else {
		return false;
	}
}
/******************************************************************************
Clear the registry of all registered cat RFID tags
******************************************************************************/
void clear_cat_registry(){
	// itterates over entire address space allocated to cat registry
	for (int registry_index = CATREGISTRYSTARTADDRESS ; registry_index < MAX_CATS_ALLOWABLE ; registry_index++){
		EEPROM.write(registry_index, 0);
	}
}

/******************************************************************************
Converts a hexadecimal value (encoded as ASCII string) to a numeric value
******************************************************************************/
long hexstr_to_value(char *str, unsigned int length) { 
	char* copy = malloc((sizeof(char) * length) + 1); 
	memcpy(copy, str, sizeof(char) * length);
	copy[length] = '\0'; 
	// the variable "copy" is a copy of the parameter "str". "copy" has an additional '\0' element to make sure that "str" is null-terminated.
	// strtol converts a null-terminated string to a long value
	long value = strtol(copy, NULL, 16); 
	 // clean up 
	free(copy);
	return value;
}

/******************************************************************************
Extracts the tag data from the rfic chip in the dedtector field
******************************************************************************/
void extract_tag() {
	// first byte of tag information
		uint8_t msg_head = rfid_tag_in_device[0];
		/* data from tag
		 10 byte => data contains 2byte version + 8byte tag
		*/
		uint8_t *msg_data = rfid_tag_in_device + 1; 
		uint8_t *msg_data_version = msg_data;
		uint8_t *msg_data_tag = msg_data + 2;
		uint8_t *msg_checksum = rfid_tag_in_device + 11; // 2 byte
		//uint8_t msg_tail = buffer[13];

		long tag = hexstr_to_value(msg_data_tag, DATA_TAG_SIZE);
	

		long checksum = 0;
		for (int i = 0; i < DATA_SIZE; i+= CHECKSUM_SIZE) {
			long val = hexstr_to_value(msg_data + i, CHECKSUM_SIZE);
			checksum ^= val;
		}


		// if in debug mode, print data to OLED/serial
		//if (DEBUG ==true){
		Serial.println(" (tag)");
		for (int i = 0; i < DATA_TAG_SIZE; ++i) {
			Serial.print(char(msg_data_tag[i]));
		}
		Serial.print("Message-Checksum (HEX): ");
		for (int i = 0; i < CHECKSUM_SIZE; ++i) {
			Serial.print(char(msg_checksum[i]));
		}
		Serial.print("Extracted Tag: ");
		Serial.println(tag);
		Serial.print("Extracted Checksum (HEX): ");
		Serial.print(checksum, HEX);
    // compare calculated checksum to retrieved checksum
		if (checksum == hexstr_to_value(msg_checksum, CHECKSUM_SIZE)) {
      // calculated checksum corresponds to transmitted checksum!
			Serial.print(" ([+] checksum OK)");
		} else {
      // checksums do not match
			Serial.print(" ([-] checksum NOT OK)");
	  }
		TAG_DATA = msg_data_tag;
		//return tag;
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
 SerialRFID.begin(9600);
 SerialRFID.listen(); 
 Serial.println("[+] INIT DONE");
}

/******************************************************************************
main loop
******************************************************************************/
void loop() {
	//read button states
	// TODO: do something stupid if all buttons pressed,
	// make led flash funny?
	int add_to_registry = digitalRead(ADDTOCATREGISTRYPIN);
	int clear_registry = digitalRead(CLEARCATREGISTRYPIN);
	int actuate_door_button = digitalRead(FLAPOPENBUTTONPIN);
	bool call_extract_tag = false;

	/*************************
	BUTTON ACTIONS
	**************************/
	// if button to register cat is pressed
	if (add_to_registry == 1);{
		set_cat(TAG_DATA);
	}
	// if button to change door position is pressed
	if (actuate_door_button == 1){
		// door is open, close door
		if (DOOROPEN == true){
		close_door();
		}
		// door is closed, open door
		else if (DOOROPEN == false){
		open_door();
		}
	}

	/***************************
	RFID DETECTION AND BEHAVIOR
	***************************/
	// if setup ran properly
	if (SerialRFID.available() > 0){

		// while reading stream from rfid scanner
		int rfid_stream = SerialRFID.read();

		// if no data was read
		if (rfid_stream == -1) { 
			Serial.println("[-] Failed to read: No data received");
			return;
		}

		// RDM630/RDM6300 found a tag => tag incoming
		if (rfid_stream == 2) {
			buffer_index = 0;
			rfid_detected = true;
			} 
		
		// tag has been fully transmitted
		else if (rfid_stream == 3) {
			// extract tag at the end of the function call
			call_extract_tag = true;
		}

		//if (check_buffer_overflow()){
		//	return;
		if (buffer_index >= BUFFER_SIZE) { 
			Serial.println("[-] EROOR: Buffer overflow detected!");
			buffer_index = 0;
		}

		//if (buffer_index >= BUFFER_SIZE) { // checking for a buffer overflow (It's very unlikely that an buffer overflow comes up!)
		//	Serial.println("Error: Buffer overflow detected! ");
		//	return;
		//}
		// everything is alright => copy current value to buffer
		rfid_tag_in_device[buffer_index++] = rfid_stream;

		if (rfid_detected == true && call_extract_tag == true) {
			if (buffer_index == BUFFER_SIZE) {
				// assigns tag data to global variable
				extract_tag();

				// check for buffer overflow 
				if (buffer_index >= BUFFER_SIZE) { 
					Serial.println("[-] EROOR: Buffer overflow detected!");
					buffer_index = 0;
				}

				// checks if cat is allowed to use feeder
				cat_allowed = check_cat(TAG_DATA);
				// cat IS allowed to use feeder
				if (cat_allowed){
					open_door();
				}
				// cat IS NOT allowed to use feeder
				// fuck off asshole
				else{
					//continue;
				}
			
			// something is wrong... start again looking for preamble (value: 2)
			} else {
				buffer_index = 0;
				return;
			}
		}		
	}		
	// if a cat walked into the device and its rfid tag matched one in the registry
}