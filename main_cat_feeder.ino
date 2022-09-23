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
static bool DEBUG true

// switch to check if its being run for the first time
bool first_run true

/******************************************************************************
servo variables
******************************************************************************/
// create servo object to control a servo
Servo food_flap;
Rdm6300 rdm6300;

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

#define ADDTOCATREGISTRYPIN 10
#define CLEARCATREGISTRYPIN 9
#define CAT_FLAP_SERVO_PIN 8
#define FLAPOPENBUTTONPIN 7
#define DEBUG_BUTTON 4

#define RFID_RX_PIN 6
#define RFID_TX_PIN 5

/******************************************************************************
RFID variables
******************************************************************************/
// trigger switch, is true when rfid tag is detected in device
bool rfid_detected;

// used to store an incoming data frame 
uint8_t rfid_tag_in_device[BUFFER_SIZE];

// rfid tag stored in eeprom
uint8_t stored_rfid_tag[BUFFER_SIZE];

// is cat allowed to use the feeder?
bool cat_allowed = false;

bool cat_still_there = false

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
#define REGISTRYSIZE = RDM6300_PACKET_SIZE * MAX_CATS_ALLOWABLE

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


/******************************************************************************
EEPROM FUNCTIONS
******************************************************************************/
uint8_t convertu32_to_u8(uint32_t number_to_convert){
	uint8_t *arr;
	//uint32_t Tx_PP= 0x02F003E7;
	arr=(uint8_t*) &number_to_convert;
	return &arr;
	}

/******************************************************************************
zeros out the eeprom space reserved for the cat registry
******************************************************************************/
void clear_eeprom(){
	for (index = 0, REGISTRYSIZE, index++){
		EEPROM.write(index, 0);
	}
}
/******************************************************************************
SET a cat's RFID tag to the registry
inputs:
	int      | address
	uint8_t  | byte array with rfid tag number
******************************************************************************/
void WriteTagToRegistry(int address, uint8_t tag_data[]){//}, int arraySize) {
	for (int i = 0; i < sizeof(tag_data); i++) {
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
	for (int i = 0; i < sizeof(rfid_tag_in_device); i++) {
		// store in global to validate against
		stored_rfid_tag[i] = (uint8_t) EEPROM.read(address+i);
	}
}

/******************************************************************************
Setter for cat registry
modify this function to implement more cats allowed
******************************************************************************/
void set_cat(uint8_t tag_bytes[]){
	WriteTagToRegistry(CATREGISTRYSTARTADDRESS,tag_bytes);
}

/******************************************************************************
Check if cat in scanner is allowed to access feeder
Compares tag in device with tag stored in EEPROM
sets tag data into global stored_rfid_tag
******************************************************************************/
bool check_cat(){
	// sets stored_rfid_tag[] with tag number from EEPROM
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
	// itterates over entire address space allocated to cat registry
	for (int registry_index = CATREGISTRYSTARTADDRESS ; registry_index < MAX_CATS_ALLOWABLE ; registry_index++){
		EEPROM.write(registry_index, 0);
	}
}

/******************************************************************************
Extracts the tag data from the rfic chip in the dedtector field
******************************************************************************/
void extract_tag() {
	/* get_new_tag_id returns the tag_id of a "new" near tag,
	following calls will return 0 as long as the same tag is kept near. */
    //rfid_tag_in_device = rdm6300.get_new_tag_id();
	if (DEBUG){
		if (rfid_tag_in_device != 0){
    		Serial.println("RFID TAG INFO:");
	  		Serial.println(rdm6300.get_tag_id(), HEX);
    	}
	}
}
void wait_for_cat_to_finish(){
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
			cat_still_there = false
		}
		else{
			delay(1000)
		}
	}
}
void check_first_run(){
	if first_run == true{
		// wait 30 seconds for PIR to normalize
		Serial.println("[+] Waiting 30 seconds for PIR to normalize")
		delay(30000)
		first_run = false
		EEPROM.
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
main loop
******************************************************************************/
void loop() {
    // read rfid
    //if (rdm6300._update()){
		rfid_tag_in_device = convertu32_to_u8(rdm6300.get_tag_id());
        //rfid_tag_in_device = rdm6300.get_tag_id();
        // set trigger
        rfid_detected = true;
    //}
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
	if (rfid_detected && add_to_registry == 1);{
		set_cat(rfid_tag_in_device);
	}
	// if button to change door position is pressed
	if (actuate_door_button == 1){
		// door is open, close door
		if (DOOROPEN == true){
		close_door();
        DOOROPEN = false;
		}
		// door is closed, open door
		else if (DOOROPEN == false){
		open_door();
        DOOROPEN = true;
		}
	}

	/***************************
	RFID DETECTION AND BEHAVIOR
	***************************/
    // returns 0 if no tag near, or in second itteration
    //rfid_tag_in_device = rdm6300.get_new_tag_id()
    // tag is near device
	if (rfid_detected){
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
			cat_still_there = true
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
		
	// something is wrong... start again looking for preamble (value: 2)
	} else {
		//return;
	}		
}	

