// Input Reader
//  Arduino code to read changes from two buttons and three digital encoders

// variables for top encoder (encoderR in Unreal)
int encoderTopPos = 0;
int encoderTopPinALast = LOW;

// variables for top button (buttonR in Unreal)
int buttonTopState = LOW;

// variables for middle encoder (encoderF in Unreal)
int encoderMidPos = 0;
int encoderMidPinALast = LOW;

// variables for bottom button (buttonL in Unreal)
int buttonBotState = LOW;

// variables for bottom encoder (encoderL in Unreal)
int encoderBotPos = 0;
int encoderBotPinALast = LOW;

// layout of pins on board
int encoderTopPinA = 11;
int encoderTopPinB = 10;
int buttonTopPin = 9;
int encoderMidPinA = 8;
// board divide here
int encoderMidPinB = 7;
int buttonBotPin = 6;
int encoderBotPinA = 5;
int encoderBotPinB = 4;

// other global variables
int readValue = LOW;
int hasChanged = 0;

void setup() {
    // setup two pins to read from top encoder
    pinMode(encoderTopPinA, INPUT);
    pinMode(encoderTopPinB, INPUT);

    // setup one pin to read from top button
    pinMode(buttonTopPin, INPUT);

    // setup two pins to read from middle encoder
    pinMode(encoderMidPinA, INPUT);
    pinMode(encoderMidPinB, INPUT);

    // setup one pin to read from bottom button
    pinMode(buttonBotPin, INPUT);

    // setup two pins to read from bottom encoder
    pinMode(encoderBotPinA, INPUT);
    pinMode(encoderBotPinB, INPUT);

    // setup serial port printing (bitrate must match Unreal bitrate)
    Serial.begin(9600);
}

void loop() {
    // assume this loop has no updates
    hasChanged = 0;

    // check for updates in top encoder position
    readValue = digitalRead(encoderTopPinA);
    if ((encoderTopPinALast == LOW) && (readValue == HIGH)) {
        if (digitalRead(encoderTopPinB) == LOW) {
            encoderTopPos--;
        }
        else {
            encoderTopPos++;
        }
        hasChanged = 1;
    } 
    encoderTopPinALast = readValue;

    // check for updates in top button press state
    readValue = digitalRead(buttonTopPin);
    if (buttonTopState != readValue) {
        buttonTopState = readValue;
        hasChanged = 1;
    }

    // check for updates in middle encoder position
    readValue = digitalRead(encoderMidPinA);
    if ((encoderMidPinALast == LOW) && (readValue == HIGH)) {
        if (digitalRead(encoderMidPinB) == LOW) {
            encoderMidPos--;
        }
        else {
            encoderMidPos++;
        }
        hasChanged = 1;
    } 
    encoderMidPinALast = readValue;

    // check for updates in bottom button press state
    readValue = digitalRead(buttonBotPin);
    if (buttonBotState != readValue) {
        buttonBotState = readValue;
        hasChanged = 1;
    }

    // check for updates in middle encoder position
    readValue = digitalRead(encoderBotPinA);
    if ((encoderBotPinALast == LOW) && (readValue == HIGH)) {
        if (digitalRead(encoderBotPinB) == LOW) {
            encoderBotPos--;
        }
        else {
            encoderBotPos++;
        }
        hasChanged = 1;
    } 
    encoderBotPinALast = readValue;

    // for updates, write button and encoder states to serial port
    // format string "buttonR,buttonL,encoderR,encoderF,encoderL;" should be written
    if (hasChanged) {
        Serial.print(buttonTopState == HIGH);
        Serial.print(",");
        Serial.print(buttonBotState == HIGH);
        Serial.print(",");
        Serial.print(encoderTopPos);
        Serial.print(",");
        Serial.print(encoderMidPos);
        Serial.print(",");
        Serial.print(encoderBotPos);
        Serial.print(";");
    }
}

