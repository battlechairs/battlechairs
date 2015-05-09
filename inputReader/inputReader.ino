// Input Reader
//  Arduino code to read changes from two buttons and three digital encoders

// event enumerations
const int ENCODER_TOP_UP = 0x001;
const int ENCODER_TOP_DOWN = 0x002;
const int BUTTON_TOP_UP = 0x004;
const int BUTTON_TOP_DOWN = 0x008;
const int ENCODER_MIDDLE_UP = 0x010;
const int ENCODER_MIDDLE_DOWN = 0x020;
const int BUTTON_BOTTOM_UP = 0x040;
const int BUTTON_BOTTOM_DOWN = 0x080;
const int ENCODER_BOTTOM_UP = 0x100;
const int ENCODER_BOTTOM_DOWN = 0x200;

// state for top encoder (encoderR in Unreal)
int encoderTopPinALast = LOW;

// state for top button (buttonR in Unreal)
int buttonTopPinLast = LOW;

// state for middle encoder (encoderF in Unreal)
int encoderMiddlePinALast = LOW;

// state for bottom button (buttonL in Unreal)
int buttonBottomPinLast = LOW;

// state for bottom encoder (encoderL in Unreal)
int encoderBottomPinALast = LOW;

// layout of pins on board
int encoderTopPinA = 13;
int encoderTopPinB = 12;
int buttonTopPin = 10;
int encoderMiddlePinA = 8;
// board divide here
int encoderMiddlePinB = 7;
int buttonBottomPin = 5;
int encoderBottomPinA = 3;
int encoderBottomPinB = 2;

// for analog buttons
const int useAnalogButtons = 1;
const int analogThreshold = 800;
int buttonTopAnalogPin = A0;
int buttonBottomAnalogPin = A2;

// analog lowpass filter variables
int buttonTopLowpass = 915;
int buttonBottomLowpass = 915;
int lowpassParts = 5;
int lowpassResponse = 4;

// other global variables
int readValue = LOW;
int event = 0;

void setup() {
    // setup two pins to read from top encoder
    pinMode(encoderTopPinA, INPUT);
    pinMode(encoderTopPinB, INPUT);

    // setup one pin to read from top button
    pinMode(buttonTopPin, INPUT);

    // setup two pins to read from middle encoder
    pinMode(encoderMiddlePinA, INPUT);
    pinMode(encoderMiddlePinB, INPUT);

    // setup one pin to read from bottom button
    pinMode(buttonBottomPin, INPUT);

    // setup two pins to read from bottom encoder
    pinMode(encoderBottomPinA, INPUT);
    pinMode(encoderBottomPinB, INPUT);

    // setup serial port printing (bitrate must match Unreal bitrate)
    Serial.begin(9600);
}

void loop() {
    // assume this loop has no events
    event = 0;

    // check for events in top encoder position
    readValue = digitalRead(encoderTopPinA);
    if ((encoderTopPinALast == LOW) && (readValue == HIGH)) {
        if (digitalRead(encoderTopPinB) == LOW) {
            event |= ENCODER_TOP_DOWN;
        }
        else {
            event |= ENCODER_TOP_UP;
        }
    } 
    encoderTopPinALast = readValue;

    if (!useAnalogButtons) {
        // check for events in top button press state
        readValue = digitalRead(buttonTopPin);
        if (buttonTopPinLast != readValue) {
            buttonTopPinLast = readValue;
            if (readValue) event |= BUTTON_TOP_DOWN;
            else event |= BUTTON_TOP_UP;
        }
    } else if (useAnalogButtons) {
        readValue = analogRead(buttonTopAnalogPin);
        buttonTopLowpass = (lowpassResponse * readValue +
                           (lowpassParts - lowpassResponse) * buttonTopLowpass)
                           / lowpassParts;
        if (buttonTopPinLast == LOW) {
            if (buttonTopLowpass < analogThreshold) {
                buttonTopPinLast = HIGH;
                event |= BUTTON_TOP_DOWN;
            }
        } else if (buttonTopPinLast == HIGH) {
            if (buttonTopLowpass >= analogThreshold) {
                buttonTopPinLast = LOW;
                event |= BUTTON_TOP_UP;
            }
        }
    }

    // check for events in middle encoder position
    readValue = digitalRead(encoderMiddlePinA);
    if ((encoderMiddlePinALast == LOW) && (readValue == HIGH)) {
        if (digitalRead(encoderMiddlePinB) == LOW) {
            event |= ENCODER_MIDDLE_DOWN;
        }
        else {
            event |= ENCODER_MIDDLE_UP;
        }
    } 
    encoderMiddlePinALast = readValue;

    if (!useAnalogButtons) {
        // check for events in bottom button press state
        readValue = digitalRead(buttonBottomPin);
        if (buttonBottomPinLast != readValue) {
            buttonBottomPinLast = readValue;
            if (readValue) event |= BUTTON_BOTTOM_DOWN;
            else event |= BUTTON_BOTTOM_UP;
        }
    } else if (useAnalogButtons) {
        readValue = analogRead(buttonBottomAnalogPin);
        buttonBottomLowpass = (lowpassResponse * readValue +
                              (lowpassParts - lowpassResponse) * buttonBottomLowpass)
                              / lowpassParts;
        if (buttonBottomPinLast == LOW) {
            if (buttonBottomLowpass < analogThreshold) {
                buttonBottomPinLast = HIGH;
                event |= BUTTON_BOTTOM_DOWN;
            }
        } else if (buttonBottomPinLast == HIGH) {
            if (buttonBottomLowpass >= analogThreshold) {
                buttonBottomPinLast = LOW;
                event |= BUTTON_BOTTOM_UP;
            }
        }
    }

    // check for events in middle encoder position
    readValue = digitalRead(encoderBottomPinA);
    if ((encoderBottomPinALast == LOW) && (readValue == HIGH)) {
        if (digitalRead(encoderBottomPinB) == LOW) {
            event |= ENCODER_BOTTOM_DOWN;
        }
        else {
            event |= ENCODER_BOTTOM_UP;
        }
    } 
    encoderBottomPinALast = readValue;

    // for any events, write combined event state and a verification hash
    // format string "event:<event>,hash:<hash>;" should be written
    if (event) {
        Serial.print("event:");
        Serial.print(event);
        Serial.print(",hash:");
        Serial.print(event+1);
        Serial.print(";");
    }
}

