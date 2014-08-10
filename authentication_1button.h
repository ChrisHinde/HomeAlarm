#define LONG  500                           // A long button press should be ~500 ms
#define SHORT 200                           // A short button press should be ~200 ms
#define TOLERANCE 100                       // Use a tolerance of +/- 100 ms (seems like a lot, but this makes the code easier to enter)

const byte PASSWORD_LENGTH = 6;             // The password/button sequence is "6 clicks long"

int secretCode[PASSWORD_LENGTH] = { LONG, SHORT, LONG, SHORT, SHORT, LONG };     // The password/secret code
//int enteredCode[PASSWORD_LENGTH];                                              // Holds the code the user entered
byte codePos = 0;
boolean correctCode = false;
unsigned long lastPressMS = 0;                                                 // Holds the timestamp of the last button press

byte lastButtonState = HIGH;
byte buttonState = HIGH;

// Initiate the authentication method
void initAuth()
{
  // This authentication method doesn't need to do any initialization
}

// Check for deactivation from user
void checkForDeactivation()
{
  //  Read the button state
  buttonState = digitalRead(BUTTON_PIN);

  // If the button state has changed (the user pressed/let go of the button)
  if ( buttonState != lastButtonState ) {
    // If the last change was was more than [Long press * 10] (5 seconds) ago
    //  we start over with the password check
    if ( millis() - lastPressMS > LONG * 10 ) {
      correctCode = false;
      codePos = 0;
    }

    // To avoid "fake presses" we do some simple debouncing by ignoring state change that's 42ms or less
    if ( millis() - lastPressMS > 42 ) {
      // Store the last button state
      lastButtonState = buttonState;
      
      // Turn off the red LED
      digitalWrite( REDLED_PIN, LOW );

      // If the button is pressed
      if ( buttonState == LOW ) {
        // Store the timestamp of the button press
        lastPressMS = millis();
      // If the button was released
      } else {
        // Calculate the length of the button press
        int btnPress = millis() - lastPressMS;

        // Check if the length of the button press matches the current code position (with tolerance)
        if ( ( btnPress > ( secretCode[codePos] - TOLERANCE ) ) ||
             ( btnPress < ( secretCode[codePos] + TOLERANCE ) ) ) {
          codePos++;                  // Advance the position the code checking
          correctCode = true;         // Indicate that the password is correct so far
          lastPressMS = millis();     // Store the timestamp of the button release
          
          // Turn turn on the green LED to indicate "correct code entered so far"
          digitalWrite( GREENLED_PIN, HIGH );
        // Otherwise the user entered an incorrect code
        } else {
          codePos = 0;                      // Reset the position for the code checking
          correctCode = false;              // Indicate that the password isn't correct
          lastPressMS = millis() - 2000;    // By setting the timestamp for last press 2 seconds in the past
                                            //  we effectively stop the code checking for 2 secends
          digitalWrite( REDLED_PIN, HIGH ); // Turn on the red LED to indicate wrong code (we could also play a melody)
        }
      }
    }
  }

  // If the user entered the whole password correctly
  if ( ( codePos == PASSWORD_LENGTH ) && correctCode ) {
    // Turn off the alarm
    deactivateAlarm();

    correctCode = false;
    codePos = 0;
  }

}

