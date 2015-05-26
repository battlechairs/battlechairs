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
int encoderTopPinA = 12;
int encoderTopPinB = 11;
int buttonTopPin = 10;
int encoderMiddlePinA = 8;
// board divide here
int encoderMiddlePinB = 7;
int buttonBottomPin = 5;
int encoderBottomPinA = 3;
int encoderBottomPinB = 2;

// for analog buttons
const int useAnalogButtons = 1;
const int analogThresholdLow = 800;
const int analogThresholdHigh = 880;
int buttonTopAnalogPin = A0;
int buttonBottomAnalogPin = A2;

// analog lowpass filter variables
int buttonTopLowpass = 915;
int buttonBottomLowpass = 915;
int lowpassParts = 2;
int lowpassResponse = 1;

// other global variables
int readValue = LOW;
int event = 0;

// input-reading variables
char incomingByte = 0;
int requestBufferPos = 0;
char requestBuffer[16];

// communication protocols
char eventLoopRequest[16] = "BC_connect;";
char listenRequest[16] = "BC_disconnect;";
char listenResponse[16] = "BC_acknowledge;";

// different behaviors of arduino
const int listenMode = 0;
const int eventLoopMode = 1;
int mode = listenMode;

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

int listenParse() {
    int iter;

    // compare input buffer with protocol
    for (iter = 0; iter < 16; iter++) {
        // if invalid, ignore
        if (requestBuffer[iter] != eventLoopRequest[iter]) {
            return listenMode;
        }
        // if valid up to ';', acknowledge and enter event loop mode
        else if (requestBuffer[iter] == ';') {
            Serial.print(listenResponse);
            return eventLoopMode;
        }
    }

    // default to listen mode
    return listenMode;
}

int eventLoopParse() {
    int iter;

    // compare input buffer with protocol
    for (iter = 0; iter < 16; iter++) {
        // if invalid, ignore
        if (requestBuffer[iter] != listenRequest[iter]) {
            return eventLoopMode;
        }
        // if valid up to ';', enter listen mode
        else if (requestBuffer[iter] == ';') {
            return listenMode;
        }
    }

    // default to event loop mode
    return eventLoopMode;
}

void loop() {
    // LISTEN MODE
    if (mode == listenMode) {
        // read every available input character
        while (Serial.available() > 0) {
            incomingByte = (char)Serial.read();

            requestBuffer[requestBufferPos] = incomingByte;
            requestBufferPos++;

            // prevent input buffer from overflowing
            if (requestBufferPos > 15) {
                requestBufferPos = 15;
            }

            // when ';' input, attempt to parse protocol
            // if request to connect, enter event loop mode
            if (incomingByte == ';') {
                mode = listenParse();
                requestBufferPos = 0;
                requestBuffer[0] = 0;
            }
        }

        // re-enter loop function
        return;
    }

    // EVENT LOOP MODE
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
            if (buttonTopLowpass < analogThresholdLow) {
                buttonTopPinLast = HIGH;
                event |= BUTTON_TOP_DOWN;
            }
        } else if (buttonTopPinLast == HIGH) {
            if (buttonTopLowpass >= analogThresholdHigh) {
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
            if (buttonBottomLowpass < analogThresholdLow) {
                buttonBottomPinLast = HIGH;
                event |= BUTTON_BOTTOM_DOWN;
            }
        } else if (buttonBottomPinLast == HIGH) {
            if (buttonBottomLowpass >= analogThresholdHigh) {
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

    // check for input while in event loop mode
    while (Serial.available() > 0) {
        incomingByte = (char)Serial.read();

        requestBuffer[requestBufferPos] = incomingByte;
        requestBufferPos++;

        // prevent input buffer from overflowing
        if (requestBufferPos > 15) {
            requestBufferPos = 15;
        }

        // when ';' input, attempt to parse protocol
        // if request to disconnect, enter listen mode
        if (incomingByte == ';') {
            mode = eventLoopParse();
            // check if connection request receieved
            listenParse();
            requestBufferPos = 0;
            requestBuffer[0] = 0;
        }
    }
}

