
#include <string.h>

#include "pitches.h" // Include the "tone/pitch definitions" (from http://arduino.cc/en/Tutorial/Tone)

const byte MELODY_COUNT = 7;

int melodies[MELODY_COUNT][8] = {
                      { 0, NOTE_C4, 0, NOTE_C4, 0, NOTE_C4, 0, NOTE_C4 },             // 1
                      { 0, NOTE_C6, 0, NOTE_C6, 0, NOTE_C6, 0, NOTE_C6 },             // 2
                      { 0, NOTE_E6, 0, NOTE_E6, 0, NOTE_E6, 0, NOTE_E6 },             // 3
                      { 0, NOTE_E7, 0, NOTE_E7, 0, NOTE_E7, 0, NOTE_E7 }, // HIGH!!   // 4
                      { 0, NOTE_A4, 0, NOTE_A4, 0, NOTE_A4, 0, NOTE_A4 },             // 5
                      { 0, NOTE_E7, 0, NOTE_E7, 0, NOTE_E7, 0, 0 }, // HIGH           // 6
                      { 0, NOTE_D6, 0, NOTE_D6, 0, NOTE_D6, 0, 0 }                    // 7
                    };
int noteDurations[MELODY_COUNT][8] = {
                           { 1000, 500, 1000, 500, 1000, 500, 1000, 500 },   // 1
                           { 600, 800, 600, 800, 600, 800, 600, 800 },       // 2
                           { 400, 800, 400, 800, 400, 800, 400, 800 },       // 3
                           { 1000, 800, 1000, 800, 1000, 800, 1000, 800 },   // 4
                           { 1000, 800, 1000, 800, 1000, 800, 1000, 800 },   // 5
                           { 50, 100, 50, 100, 50, 100, 5000, 1000 },        // 6
                           { 100, 330, 100, 300, 100, 300, 10000, 10000 },   // 7
                         };

byte currentMelody   = 0;                   // Holds the "position" current melody
byte currentNote     = 0;                   // Holds the "position" of the current note
unsigned long lastMS = 0;                   // Holds the timestamp of the last "tone change"

// Blink sequences for the LEDs, the pattern is on-off-on-off-on-off-on-off
int blinkSequences[8][8] = {
                            { 10000, 0, 10000, 0, 10000, 0, 10000, 0  },      // 1 - "Always" on
                            { 200, 1800, 200, 1800, 200, 1800, 200, 1800 },   // 2
                            { 150, 7850, 150, 7850, 150, 7850, 150, 7850 },   // 3
                            { 50, 450, 50, 450, 50, 450, 50, 450 }            // 4
                          };

byte currentRedSequence      = 0;            // Holds the current sequence for the red LED
byte currentGreenSequence    = 0;            // Holds the current sequence for the green LED
byte currentRedSequencePos   = 0;            // Holds the current position for the red LED sequence
byte currentGreenSequencePos = 0;            // Holds the current position for the green LED sequence
unsigned long redLastBlink   = 0;            // Holds the timestamp of the last red LED change
unsigned long greenLastBlink = 0;            // Holds the timestamp of the last green LED change
//boolean redLEDOn   = false;                  // Holds the state of the red LED
//boolean greenLEDOn = false;                  // Holds the state of the green LED

byte m_x = 0;                                // State variable for the "extra melody"
float m_y  = 0;                                // State variable for the "extra melody"


// Play a note on the siren
void playNote( int note ) {
  tone( SIREN_PIN, note );
}

// Play a "melody" progrmmatically instead of from a list of notes (this "melody" is hard to do in another way)
void playExtraMelody_1() {
  if ( m_y < 200 ) {
    m_y = 200;
    m_x = 1;
  }
  
  if ( millis() - lastMS > 2 ) {
    if ( m_y >= 1200 ) {
      m_y = 200;
      m_x++;
      if ( m_x >= 3 ) {
        m_x = 1;
      }
    }
    
    playNote( m_x * m_y  );
    m_y++;
    
    lastMS = millis();
  }
}

// Play a "melody" progrmmatically instead of from a list of notes (this "melody" is hard to do in another way)
void playExtraMelody_2() {
  if ( millis() - lastMS > 5 ) {
    if ( m_x >= 360 ) {
      m_x = 0;
    }
    m_y = 800 + sin( ((float)m_x * PI) / 180 ) * 500;
    playNote( m_y );
    
    m_x++;
    
    lastMS = millis();
  }
}

// Function that plays the melody (should be called once per "main loop")
void playTheMelody() {
  // currentMelody = 0 means that we shouldn't play a melody
  if ( currentMelody == 0 ) {
    return;
  }
  
  byte cMelody = currentMelody - 1;

  // If we are asked to play a melody that isn't in the list, assume that we should play the "extra melody"
  if ( cMelody == MELODY_COUNT ) {
    playExtraMelody_1();
  } else if ( cMelody >= MELODY_COUNT + 1 ) {
    playExtraMelody_2();
  // If we have played the current note long enough
  } else if ( millis() - lastMS > noteDurations[cMelody][currentNote] ) {
    // Move to next note
    currentNote++;
    // We don't have more than 8 notes, so start from the beginning again
    if ( currentNote == 8 ) currentNote = 0;

    // if the current note is 0, play nothing
    if ( melodies[cMelody][currentNote] == 0 ) {
      noTone( SIREN_PIN );
    // Otherwise, play the note
    } else {
      playNote( melodies[cMelody][currentNote] );
    }
    
    // Store the current millisecond value, so we know when we should play the next note
    lastMS = millis();
  }
}

// Sets the current melody
void playMelody( byte melody ) {
  noTone( SIREN_PIN );
  currentMelody = melody;
  currentNote   = 0;
  lastMS = 0;
  m_x = m_y = 0;
}

// Stop the current melody
void stopMelody( ) {
  currentMelody = 0;
  currentNote   = 0;
  noTone( SIREN_PIN );
}


// Stop the blink sequence for the green LED
void stopGreenBlink( ) {
  currentGreenSequence    = 0;
  currentGreenSequencePos = 0;
  
  digitalWrite( GREENLED_PIN, LOW );
}
// Stop the blink sequence for the red LED
void stopRedBlink( ) {
  currentRedSequence    = 0;
  currentRedSequencePos = 0;

  digitalWrite( REDLED_PIN, LOW );
}

// set the blink sequence for the green LED
void startGreenBlink( byte sequence ) {
  if ( sequence == 0 ) {
    stopGreenBlink();
    return;
  }

  currentGreenSequence    = sequence;
  currentGreenSequencePos = 0;
}
// set the blink sequence for the red LED
void startRedBlink( byte sequence ) {
  if ( sequence == 0 ) {
    stopRedBlink();
    return;
  }

  currentRedSequence    = sequence;
  currentRedSequencePos = 0;
}

// Blinks the green LED every eight seconds
void blinkGreenLED() {
  // If the current sequence is 0, do nothing
  if ( currentGreenSequence == 0 ) {
    return;
  }

  // If enough time has past..
  if ( millis() - greenLastBlink >= blinkSequences[currentGreenSequence - 1][currentGreenSequencePos] ) {
    do {
      // Advance the current sequence position
      currentGreenSequencePos++;
      // If the current position is 8, start from the beginning
      if ( currentGreenSequencePos >= 8 ) currentGreenSequencePos = 0;
    // If the current sequence position hasa length of 0, go to the next position
    } while ( blinkSequences[currentGreenSequence - 1][currentGreenSequencePos] == 0 );
    
    // If the sequence position is even..
    if ( currentGreenSequencePos % 2 == 0 ) {
      // .. turn the green LED on
      digitalWrite( GREENLED_PIN, HIGH );
    } else {
      // otherwise, turn it off
      digitalWrite( GREENLED_PIN, LOW );
    }
    
    greenLastBlink = millis();
  }
}

// Blinks the green LED every two seconds
void blinkRedLED(  ) {
  // If the current sequence is 0, do nothing
  if ( currentRedSequence == 0 ) {
    return;
  }

  // If enough time has past..
  if ( millis() - redLastBlink >= blinkSequences[currentRedSequence - 1][currentRedSequencePos] ) {
    do {
      // Advance the current sequence position
      currentRedSequencePos++;
      // If the current position is 8, start from the beginning
      if ( currentRedSequencePos >= 8 ) currentRedSequencePos = 0;
    // If the current sequence position hasa length of 0, go to the next position
    } while ( blinkSequences[currentRedSequence - 1][currentRedSequencePos] == 0 );
    
    // If the sequence position is even..
    if ( currentRedSequencePos % 2 == 0 ) {
      // .. turn the red LED on
      digitalWrite( REDLED_PIN, HIGH );
    } else {
      // otherwise, turn it off
      digitalWrite( REDLED_PIN, LOW );
    }
    
    redLastBlink = millis();
  }
}


/** Functions that handles incoming serial communication **/

#if SERIAL
char incomingByte     = 0;
/*char serialtmp[128]   = "";
char serialline[128]  = "";*/
String serialline_s  = "";/*
int serialline_length = 0;
int serialtmp_length  = 0;*/
int lastSend = 0;

// Sends a message via serial periodically
void sendMsg( char* msg, int wait ) {
  // If it has been more than 2 seconds since we sent our last message
  if ( millis() - lastSend > wait ) {
    Serial.print(">");
    Serial.println( msg );
    lastSend = millis();
  }
}
/*
// Sends a message via serial periodically
void sendMsgVal( char* msg, int val, int wait ) {
  // If it has been more than 2 seconds since we sent our last message
  if ( millis() - lastSend > wait ) {
    Serial.print(">");
    Serial.print( msg );
    Serial.println( val );
    lastSend = millis();
  }
}*/

boolean
collectFromSerial()
{
  boolean gotit = false;

  // Check if there's data available
  while (Serial.available() > 0 && ! gotit) {
    // Read  the incoming byte
    incomingByte = Serial.read();

    // If it's a character
    if ( ( incomingByte > 31 ) && ( incomingByte < 127 ) ) {
      // Store it
      serialline_s.concat(incomingByte);
      //serialtmp[serialtmp_length] = incomingByte;
    // If it's #10 (newline) or #0 (\0 / NULL character) 
    } else if ( ( incomingByte == 10 ) || ( incomingByte == 0 ) ) {
      // We have a full string
      gotit = true;
    }

/*  // REMOVE THIS LINE TO ACCEPT ;; AS "LINE/COMMAND ENDING"
    // If the last two characters were ;;
    if ( ( incomingByte == 59 ) && ( serialtmp[serialtmp_length-2] == 59 ) ) {
      // Stop reading
      serialtmp_length -= 2;
      gotit = true;
    }/**/
  }

  // Returns true if we got a EOL
  return gotit;
}

void clearSerial() {
  serialline_s = "";
}
#endif
