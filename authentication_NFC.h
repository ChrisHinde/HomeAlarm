
/////////////////////////////////////////////////////////////////////
//set the pin
/////////////////////////////////////////////////////////////////////
const int chipSelectPin = 10;
const int NRSTPD = 9;

#include <SPI.h>

const byte NUM_OF_IDS = 5;
const byte MAX_ID_LENGTH = 7;
byte valid_ids[NUM_OF_IDS][7] = { { 0x05, 0x83, 0x71, 0x06, 0x00, 0x00, 0x00 }, // ID #0
                                  { 0x04, 0x88, 0x41, 0x32, 0x23, 0x35, 0x80 }, // ID #1 
                                  { 0x04, 0xEF, 0x46, 0x32, 0x23, 0x35, 0x80 }, // ID #2 
                                  { 0x04, 0x14, 0x79, 0x32, 0x23, 0x35, 0x80 }, // ID #3 
                                  { 0x04, 0x9A, 0x46, 0x32, 0x23, 0x35, 0x80 }, // ID #4
                               };

#include "NFC.h"


boolean correctCode = false;


// Initiate the authentication method
void initAuth()
{
  SPI.begin();
  
  pinMode(chipSelectPin,OUTPUT);             // Set digital pin 10 as OUTPUT to connect it to the RFID /ENABLE pin 
  digitalWrite(chipSelectPin, LOW);          // Activate the RFID reader
  pinMode(NRSTPD,OUTPUT);                    // Set digital pin 10 , Not Reset and Power-down
  digitalWrite(NRSTPD, HIGH);
  
  MFRC522_Init();  
}

// Check for deactivation from user
void checkForDeactivation()
{
  byte idLength = 0;
  byte cardType = readCardID( idLength );

  if ( cardType != CT_NONE ) {
#if SERIAL
    if ( cardType == CT_CARD ) {
      Serial.print("Found card: ");
    } else if ( cardType == CT_TAG ) {
      Serial.print("Found tag: ");
    } else {
      Serial.println("Found ????: ");
    }
    dumpID( (byte*)serNum, idLength );
    Serial.println("");
#endif
    
    byte id = 0;
    if ( (id = checkID( idLength )) != CT_NONE ) {
      digitalWrite( GREENLED_PIN, HIGH );

#if SERIAL
      Serial.print("Valid ID#:");
      Serial.print(id);
      Serial.println("");
#endif

      // Turn off the alarm
      deactivateAlarm();
    } else {
      digitalWrite( REDLED_PIN, HIGH );
#if SERIAL
      Serial.println( "Invalid ID!\n" );
#endif
    }
  }

}

