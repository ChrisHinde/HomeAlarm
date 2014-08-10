#include <string.h>


/* CONSTANTS USED IN THE CODE */

//#define SERIAL 1                       // Comment this row if you don't want any Serial communication

// Constants for the alarm states
#define STANDBY 0
#define ACTIVATING 1
#define ACTIVATED 2
#define TRIPPED_GRACE 4
#define TRIPPED 8
#define DEACTIVATING 16


#define SENSOR_PIN     7                 // The Magnetic sensor is connected to pin 7
#define BUTTON_PIN     8                 // The Button is connected to pin 8

#define SIREN_PIN      5                 // The "Siren"/piezo is connected to pin 5 (PWM)

#define REDLED_PIN     3                 // The Green LED is connected to pin 3
#define GREENLED_PIN   2                 // The Green LED is connected to pin 2


const int GRACE_PERIOD = 30000;            // The Person who opened the door has 30 seconds to disable the alarm

const int BUTTON_ACTIVATION_TIME = 3000;   // Press the button for 3 seconds to activate the alarm

const int DEACTIVATION_DELAY = 5000;       // Wait 5 seconds after deactivation  

/* /CONSTANTS */

/* GLOBAL VARIABLES */

unsigned long tripped        = 0;            // Holds the timestamp of when the alarm was tripped
unsigned long buttonPress    = 0;            // Holds the timestamp of the beginning of the button press

byte sensorState = digitalRead(SENSOR_PIN);  // Holds the state of the sensor  

byte alarmState = STANDBY;

/* /GLOBAL VARIABLES */


// Include some helper functions
#include "lib.h"


/** AUTHENTICATION METHOD **/

  /* Select the authentication method by
     uncommenting the corresponding include and commenting the others.
     
     If you uncomment more than one you will get an error when you compile the code!
  */

//#include "authentication_1button.h"    // "1 button" authentication
//#include "authentication_1button_2.h"    // "1 button" authentication - variant 2
//#include "authentication_rotEnc.h"       // roatry encoding authentication

#include <SPI.h>
#include "authentication_NFC.h"        // NFC authentication

/** /AUTHENTICATION METHOD **/

void setup() {

  // Set the sensor and button pins as inputs (and activate the pullup)
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Set the LED pins as outputs
  pinMode(REDLED_PIN,   OUTPUT);
  pinMode(GREENLED_PIN, OUTPUT);

  // Set the "Siren" pin as output
  pinMode(SIREN_PIN,   OUTPUT);
  
  // Init the auth method
  initAuth();

#if SERIAL
    // initialize serial communication for debugging
    Serial.begin(9600);
    // Say Hello!
    Serial.println("Starting alarm!");
#endif
  
  // Start the "standby blink"
  startGreenBlink( 3 );
}

// the loop routine runs over and over again forever:
void loop() {
#if SERIAL
  checkSerial();
#endif

  // Check if the alarm was tripped
  if ( ( alarmState == TRIPPED_GRACE ) || ( alarmState == TRIPPED ) ) {
    alarmTripped();
  // Check if the alarm is active
  } else if ( alarmState == ACTIVATED ) {
    alarmActive();
  // Check if the alarm is deactivating
  } else if ( alarmState == DEACTIVATING ) {
    alarmDeactivating();
  // Check if the alarm is activating
  } else if ( alarmState == ACTIVATING ) {
    alarmActivating();
  // The alarm isn't active (i.e. standby)
  } else {
    alarmNotActive();
  }

  // Do the "background stuff"
  playTheMelody();
  blinkGreenLED();
  blinkRedLED();
}

#if SERIAL
// This function checks incoming commands
void checkSerial() {
  // If we got something via serial
  if ( collectFromSerial() ) {
    Serial.print( F(">") );
    Serial.println( serialline_s );
    // If the command starts with M (Melody command)
    if ( serialline_s.startsWith("M") ) {
      // Play the requested melody
      playMelody( serialline_s.charAt(1) - '0' );
      Serial.print( F("Playing melody #") );
      Serial.println( currentMelody );
    // If the command starts with B (Blink command)
    } else if ( serialline_s.startsWith("B") ) {
      // If the next character is G (Green)
      if ( serialline_s.charAt(1) == 'G' ) {
        // Start the requested green blink sequence
        startGreenBlink( serialline_s.charAt(2) - '0' );
        Serial.print( F("Blinking green LED with sequence #") );
        Serial.println( currentGreenSequence );
      // If the next character is R (Red)
      } else if ( serialline_s.charAt(1) == 'R' ) {
        // Start the requested red blink sequence
        startRedBlink( serialline_s.charAt(2) - '0' );
        Serial.print( F("Blinking red LED with sequence #") );
        Serial.println( currentRedSequence );
      }
    // If we got asked what the status is
    } else if ( serialline_s == "STATUS" ) {
      sendStatus();
    } else if ( serialline_s == "SENSORS" ) {
      sendSensors();
    // If we got the activation command
    } else if ( serialline_s == "ACTIVATE" ) {
      activateAlarm();
    // If we got the deactivation command
    } else if ( serialline_s == "DEACTIVATE" ) {
      deactivateAlarm();
    // If we got the trip command
    } else if ( serialline_s == "TRIP" ) {
      tripAlarm();
    }
    
    clearSerial();
  }
}
#endif

// What to do when the alarm is deactivating
void alarmDeactivating() {

    // Has the deactivation period passed?
    if ( millis() - tripped > DEACTIVATION_DELAY ) {
      // Switch to standby state
      alarmState = STANDBY;

      // Stop blinknig etc
      stopRedBlink( );
      stopMelody();

      // Start the "standby blink"
      startGreenBlink( 3 );
    }
}

// What to do when the alarm is activating
void alarmActivating() {
  // Has the activation period passed?
  if ( millis() - buttonPress > GRACE_PERIOD ) {
    // Set the state to ACTIVATED
    alarmState = ACTIVATED;

    // Stop the activation melody
    stopMelody();
    
    // Turn off the green LED
    stopGreenBlink( );
    
    // Blink the RED blink, to indicate that the alarm is active
    startRedBlink( 2 );
  }
}

// What to do when the alarm isn't active
void alarmNotActive() {
  // Read the state of the button
  byte buttonState = digitalRead(BUTTON_PIN);

  // Check if the button is pressed (LOW since we use pullup)
  if ( buttonState == LOW ) {
    // If the button wasn't pressed before
    if ( buttonPress == 0 ) {
      // Store the start of the button press
      buttonPress = millis();

#if SERIAL
      Serial.println( F("Button pressed!") );
#endif

      // Turn on the green LED
      startGreenBlink( 1 );

    // If the button was pressed before
    } else {
      // If the button has been pressed long enough to activate 
      if ( millis() - buttonPress > BUTTON_ACTIVATION_TIME ) {
        activateAlarm();
      }
    }
  // The button isn't pressed, so do nothing...
  } else {
    buttonPress = 0;
  }
}

// Function that activates the alarm
void activateAlarm() {
  // Set the current state
  alarmState = ACTIVATING;
  // Store the timestamp for the start of the activation
  buttonPress = millis(); // Reuse variable for activation

#if SERIAL
    Serial.print( F("Activated, Will wait for ") );
    Serial.println( GRACE_PERIOD );
#endif

  // Turn on the green LED
  startGreenBlink( 1 );
  
  // Play the activation alarm
  playMelody( 1 );
}

// What to do if someone tripped the alarm
void alarmTripped() {
  // Check how long ago the alarm was tripped to see if it's in "the grace period")
  if ( millis() - tripped < GRACE_PERIOD ) {
#if SERIAL
    sendMsg("ALARM TRIPPED: GRACE!", GRACE_PERIOD);
#endif
  // If the grace period has passed
  } else if (alarmState == TRIPPED_GRACE ) {
    alarmState = TRIPPED;

    // Turn on the red LED
    startRedBlink( 1 );

    // Start the siren
    playMelody( MELODY_COUNT );

#if SERIAL
    Serial.println("ALARM TRIPPED!");
#endif
  }

  // Check for deactivation from user
  checkForDeactivation();
}

// What to do when the alarm is active
void alarmActive() {
   // check the sensor
  sensorState = digitalRead(SENSOR_PIN);

  // If the sensor state is low/0 then the door is closed
  if ( sensorState == LOW ) {     // Active == LOW since we use a pullup and connect the sensor to GND

  // Otherwise, someone tripped the alarm
  } else {
    tripAlarm();
  }

  // Check for deactivation from user
  checkForDeactivation();
}

// Function that "trips" the alarm
void tripAlarm() {
  // Store when the alarm was tripped
  tripped = millis();
  alarmState = TRIPPED_GRACE;
  
  playMelody( 4 );

#if SERIAL
  Serial.println(F("ALARM TRIPPED - GRACE PERIOD!"));
#endif

  // Turn on the red LED
  startRedBlink( 1 );
  stopGreenBlink();
}

// Function to deactivate the alarm
void deactivateAlarm() {
  // Turn off the alarm
  alarmState = DEACTIVATING;
  // Store the timestamp for deactivation
  tripped = millis(); // Reuse the tripped variable

#if SERIAL
  Serial.println( F("ALARM DEACTIVATING!") );
#endif

  // Stop the siren and beep for deactivation
  playMelody( 6 );

  // Stop the blinking
  stopRedBlink();
  // Turn on the green LED
  startGreenBlink( 1 );
}


#if SERIAL
void sendStatus() {
  Serial.print( F("Status:") );
  
  if ( alarmState == STANDBY ) {
    Serial.println( F("STANDBY") );
  } else if ( alarmState == ACTIVATING ) {
    Serial.println( F("ACTIVATING") );
  } else if ( alarmState == ACTIVATED ) {
    Serial.println( F("ACTIVATED") );
  } else if ( alarmState == TRIPPED_GRACE ) {
    Serial.println( F("TRIPPED_GRACE") );
  } else if ( alarmState == TRIPPED ) {
    Serial.println( F("TRIPPED") );
  } else if ( alarmState == DEACTIVATING ) {
    Serial.println( F("DEACTIVATING") );
  }
  
  Serial.print( F("MELODY:") );
  Serial.println( currentMelody );
  Serial.print( F("GREEN LED:") );
  Serial.println( currentGreenSequence );
  Serial.print( F("RED LED:") );
  Serial.println( currentRedSequence );
}

void sendSensors() {
  Serial.print( "DOOR:" );
  Serial.println( digitalRead( SENSOR_PIN ) );
}
#endif
