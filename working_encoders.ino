#include <Keyboard.h>

// Multiplexer control pins
const int S0 = 4;    
const int S1 = 5;
const int S2 = 6;
const int S3 = 7;
const int SIG_A = 2; // First mux signal
const int SIG_B = 3; // Second mux signal

// Arrays to store parameters for each encoder
const uint8_t keyPositive[8] = {'1', '2', '3', '4', '5', '6', '7', '8'}; // Safe ASCII values
const uint8_t keyNegative[8] = {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i'}; // Safe ASCII values
const int threshold = 3;  

// State tracking for each encoder
uint8_t lastState[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int8_t encoderPos[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long lastKeyPress[8] = {0, 0, 0, 0, 0, 0, 0, 0};  // Debounce timing
const unsigned long debounceDelay = 50;  // Milliseconds between allowed keypresses

void setup() {
  // Configure multiplexer control pins
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  // Configure multiplexer signal pins
  pinMode(SIG_A, INPUT_PULLUP);
  pinMode(SIG_B, INPUT_PULLUP);

  // Start Serial for debugging
  Serial.begin(9600);

  // Start the Keyboard library
  Keyboard.begin();
}

// Function to set the multiplexer channel
void setMuxChannel(uint8_t channel) {
  digitalWrite(S0, channel & 0x01);
  digitalWrite(S1, (channel >> 1) & 0x01);
  digitalWrite(S2, (channel >> 2) & 0x01);
  digitalWrite(S3, (channel >> 3) & 0x01);
  delayMicroseconds(1); // Allow time for the mux to settle
}

void loop() {
  unsigned long currentTime = millis(); // Get current time for debounce tracking

  for (uint8_t i = 0; i < 8; i++) {
    setMuxChannel(i); // Select the correct encoder

    // Read the encoder signal pins
    uint8_t pinA = digitalRead(SIG_A);
    uint8_t pinB = digitalRead(SIG_B);
    uint8_t currentState = (pinA << 1) | pinB;

    // Debugging output
    Serial.print("Encoder ");
    Serial.print(i);
    Serial.print(" State: ");
    Serial.print(currentState, BIN);
    Serial.print(" Pos: ");
    Serial.println(encoderPos[i]);

    // Check for state change
    if (currentState != lastState[i]) {
      // Handle clockwise rotation
      if ((lastState[i] == 0b00 && currentState == 0b01) ||
          (lastState[i] == 0b01 && currentState == 0b11) ||
          (lastState[i] == 0b11 && currentState == 0b10) ||
          (lastState[i] == 0b10 && currentState == 0b00)) {
        encoderPos[i]++;
      }
      // Handle counter-clockwise rotation
      else if ((lastState[i] == 0b00 && currentState == 0b10) ||
               (lastState[i] == 0b10 && currentState == 0b11) ||
               (lastState[i] == 0b11 && currentState == 0b01) ||
               (lastState[i] == 0b01 && currentState == 0b00)) {
        encoderPos[i]--;
      }

      lastState[i] = currentState; // Update the last state

      // Check if the encoder has moved enough and debounce time has passed
      if (abs(encoderPos[i]) >= threshold) {
        if (currentTime - lastKeyPress[i] >= debounceDelay) {
          // Send the appropriate key
          if (encoderPos[i] > 0) {
            Keyboard.write(keyPositive[i]);
            Serial.print("Key Positive: ");
            Serial.println((char)keyPositive[i]);
          } else {
            Keyboard.write(keyNegative[i]);
            Serial.print("Key Negative: ");
            Serial.println((char)keyNegative[i]);
          }
          lastKeyPress[i] = currentTime; // Update last keypress time
          encoderPos[i] = 0; // Reset the encoder position
        }
      }
    }
  }
}
