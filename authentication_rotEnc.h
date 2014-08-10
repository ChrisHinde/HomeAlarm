
const int encoderPinA = 3;
const int encoderPinB = 4;
const int encoderPos  = 0;

int encoderPinALast = LOW;

boolean correctCode = false;


// Initiate the authentication method
void initAuth()
{
  pinMode (encoderPinA, INPUT);
  pinMode (encoderPinB, INPUT);
}

// Check for deactivation from user
void checkForDeactivation() {

  // If the user entered the whole password correctly
  if ( correctCode ) {
    // Turn off the alarm
    deactivateAlarm();
  }

}

