// Event Push
//  Arduino code to read changes from one button and two digital encoders

// variables for encoder 1 (encoderL in Unreal)
int encoder1PinA = 8;
int encoder1PinB = 9;
int encoder1Pos = 0;
int encoder1PinALast = LOW;

// variables for button (button in Unreal)
int buttonPin = 10;
int buttonState = 0;
int buttonLast = 0;

// variables for encoder 0 (encoderR in Unreal)
int encoder0PinA = 11;
int encoder0PinB = 12;
int encoder0Pos = 0;
int encoder0PinALast = LOW;

// other global variables
int n = LOW;
int hasChanged;

void setup() {
    // setup two pins to read from encoder 1
    pinMode(encoder1PinA,INPUT);
    pinMode(encoder1PinB,INPUT);

    // setup one pin to read from button
    pinMode(buttonPin,INPUT);

    // setup two pins to read from encoder 0
    pinMode(encoder0PinA,INPUT);
    pinMode(encoder0PinB,INPUT);

    // setup serial port printing (bitrate must match Unreal bitrate)
    Serial.begin(9600);
}

void loop() {
    // assume this loop has no updates
    hasChanged = 0;

    // check for updates in encoder 1 position
    n = digitalRead(encoder1PinA);
    if ((encoder1PinALast == LOW) && (n == HIGH)) {
        if (digitalRead(encoder1PinB) == LOW) {
            encoder1Pos--;
        }
        else {
            encoder1Pos++;
        }
        hasChanged = 1;
    } 
    encoder1PinALast = n;

    // check for updates in button press state
    n = digitalRead(buttonPin);
    if (buttonState != n) {
        buttonState = n;
        hasChanged = 1;
    }

    // check for updates in encoder 0 position
    n = digitalRead(encoder0PinA);
    if ((encoder0PinALast == LOW) && (n == HIGH)) {
        if (digitalRead(encoder0PinB) == LOW) {
            encoder0Pos--;
        }
        else {
            encoder0Pos++;
        }
        hasChanged = 1;
    }
    encoder0PinALast = n;

    // for updates, write button and encoder states to serial port
    // format string "button,encoderR,encoderL;" should be written
    if (hasChanged) {
        Serial.print(buttonState == HIGH);
        Serial.print(",");
        Serial.print(encoder0Pos);
        Serial.print(",");
        Serial.print(encoder1Pos);
        Serial.print(";");
    }
}

