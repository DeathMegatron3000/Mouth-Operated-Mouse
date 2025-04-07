// Arduino Uno Mouth-Operated Mouse - Serial Communication Version
// This code reads sensor data and sends commands via Serial
#import <unoHID.h>

// Pressure sensor sampling values
const byte SAMPLE_LENGTH = 8;       // Number of samples to average
volatile byte sampleCounter = 0;
volatile int samples[SAMPLE_LENGTH];
unsigned long sampleTimer = 0;
const byte SAMPLE_PERIOD = 10;      // Time between samples in milliseconds

// Cursor movement variables
const int CURSOR_UPDATE_PERIOD = 10; // Time between cursor updates in milliseconds
unsigned long cursorTimer = 0;

// Joystick variables
const int JOYSTICK_DEADZONE = 10;    // Ignore joystick movements smaller than this
int joystickX = 0;
int joystickY = 0;

// Pressure thresholds - these will need calibration for your specific sensor
const int HARD_SIP_THRESHOLD = 360;  // Threshold for right click
const int SOFT_SIP_THRESHOLD = 410;  // Threshold for scroll down
const int NEUTRAL_MIN = 460;         // Lower bound of neutral zone
const int NEUTRAL_MAX = 550;         // Upper bound of neutral zone
const int SOFT_PUFF_THRESHOLD = 600; // Threshold for scroll up
const int HARD_PUFF_THRESHOLD = 650; // Threshold for left click

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("Arduino Mouth-Operated Mouse - Serial Version");
  
  // Initialize timers
  sampleTimer = millis() + SAMPLE_PERIOD;
  cursorTimer = millis() + CURSOR_UPDATE_PERIOD;
}

void loop() {
  // Sample pressure sensor at regular intervals
  if (millis() >= sampleTimer) {
    samplePressure();
    sampleTimer = millis() + SAMPLE_PERIOD;
  }
  
  // Process pressure samples when we have enough
  if (sampleCounter >= SAMPLE_LENGTH) {
    processPressure(calculateAveragePressure());
    sampleCounter = 0;
  }
  
  // Update cursor position at regular intervals
  if (millis() >= cursorTimer) {
    updateCursorPosition();
    cursorTimer = millis() + CURSOR_UPDATE_PERIOD;
  }
}

// Sample the pressure sensor
void samplePressure() {
  samples[sampleCounter] = analogRead(A0);
  sampleCounter++;
}

// Calculate the average pressure from samples
int calculateAveragePressure() {
  long sum = 0;
  for (byte i = 0; i < SAMPLE_LENGTH; i++) {
    sum += samples[i];
  }
  return sum / SAMPLE_LENGTH;
}

// Process pressure reading and send commands via Serial
void processPressure(int pressure) {
  // Hard sip - right click
  if (pressure < HARD_SIP_THRESHOLD) {
    Serial.println("RIGHT_CLICK_DOWN");
  } 
  // Soft sip - scroll down
  else if (pressure >= HARD_SIP_THRESHOLD && pressure < NEUTRAL_MIN) {
    Serial.println("SCROLL_DOWN");  // Re-enabled scroll down functionality
  } 
  // Neutral zone
  else if (pressure >= NEUTRAL_MIN && pressure <= NEUTRAL_MAX) {
    Serial.println("NEUTRAL");
  } 
  // Soft puff - scroll up
  else if (pressure > NEUTRAL_MAX && pressure <= SOFT_PUFF_THRESHOLD) {
    Serial.println("SCROLL_UP");
  } 
  // Hard puff - left click
  else if (pressure > HARD_PUFF_THRESHOLD) {
    Serial.println("LEFT_CLICK_DOWN");
  }
}

// Read joystick and send cursor position via Serial
void updateCursorPosition() {
  // Read joystick values
  joystickX = analogRead(A1) - 512; // Center at 0
  joystickY = analogRead(A2) - 512; // Center at 0
  
  // Apply deadzone
  if (abs(joystickX) < JOYSTICK_DEADZONE) joystickX = 0;
  if (abs(joystickY) < JOYSTICK_DEADZONE) joystickY = 0;
  
  // Only send data if joystick is moved
  if (joystickX != 0 || joystickY != 0) {
    // Send joystick values as CSV
    Serial.print("MOVE,");
    Serial.print(joystickX);
    Serial.print(",");
    Serial.println(joystickY);
  }
}
