
 //Arduino Mouth-Operated Mouse
 //Based on "The 'Sup" open-source project by Jacob Field
 
 //This code implements a simple mouth-operated mouse using:
 //- Arduino Uno or Pro Micro with ATmega32u4 (required for USB HID functionality)
 //- MPXV7002DP or similar pressure sensor for sip/puff detection
 //- Analog joystick module for cursor movement
 
#include <Mouse.h>

// Pressure sensor sampling values
const byte SAMPLE_LENGTH = 8;       // Number of samples to average (smaller for faster response)
volatile byte sampleCounter = 0;
volatile int samples[SAMPLE_LENGTH];
unsigned long sampleTimer = 0;
const byte SAMPLE_PERIOD = 10;      // Time between samples in milliseconds

// Mouse button state trackers
bool isLeftPressed = false;
bool isRightPressed = false;

// Cursor movement variables
const int CURSOR_UPDATE_PERIOD = 30; // Time between cursor updates in milliseconds
unsigned long cursorTimer = 0;
const int CURSOR_SPEED = 10;         // Maximum cursor speed in pixels per update

// Joystick variables
const int JOYSTICK_DEADZONE = 10;    // Ignore joystick movements smaller than this
int joystickX = 0;
int joystickY = 0;

// Pressure thresholds - these will need calibration for your specific sensor
const int HARD_SIP_THRESHOLD = 400;  // Threshold for right click
const int SOFT_SIP_THRESHOLD = 450;  // Threshold for scroll down
const int NEUTRAL_MIN = 480;         // Lower bound of neutral zone
const int NEUTRAL_MAX = 530;         // Upper bound of neutral zone
const int SOFT_PUFF_THRESHOLD = 580; // Threshold for scroll up
const int HARD_PUFF_THRESHOLD = 630; // Threshold for left click

void setup() {
  // Initialize serial communication for debugging (optional)
  Serial.begin(9600);
  
  // Initialize the Mouse library
  Mouse.begin();
  
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

// Process pressure reading and perform mouse actions
void processPressure(int pressure) {
  // For debugging
  Serial.print("Pressure: ");
  Serial.println(pressure);
  
  // Hard sip - right click
  if (pressure < HARD_SIP_THRESHOLD) {
    if (!isRightPressed) {
      Mouse.press(MOUSE_RIGHT);
      isRightPressed = true;
      Serial.println("Right click pressed");
    }
  } 
  // Soft sip - scroll down
  else if (pressure >= HARD_SIP_THRESHOLD && pressure < NEUTRAL_MIN) {
    Mouse.move(0, 0, -1);
    Serial.println("Scroll down");
  } 
  // Neutral zone - release buttons
  else if (pressure >= NEUTRAL_MIN && pressure <= NEUTRAL_MAX) {
    if (isLeftPressed) {
      Mouse.release(MOUSE_LEFT);
      isLeftPressed = false;
      Serial.println("Left click released");
    }
    if (isRightPressed) {
      Mouse.release(MOUSE_RIGHT);
      isRightPressed = false;
      Serial.println("Right click released");
    }
  } 
  // Soft puff - scroll up
  else if (pressure > NEUTRAL_MAX && pressure <= SOFT_PUFF_THRESHOLD) {
    Mouse.move(0, 0, 1);
    Serial.println("Scroll up");
  } 
  // Hard puff - left click
  else if (pressure > HARD_PUFF_THRESHOLD) {
    if (!isLeftPressed) {
      Mouse.press(MOUSE_LEFT);
      isLeftPressed = true;
      Serial.println("Left click pressed");
    }
  }
}

// Read joystick and update cursor position
void updateCursorPosition() {
  // Read joystick values (adjust pins as needed for your wiring)
  joystickX = analogRead(A1) - 512; // Center at 0
  joystickY = analogRead(A2) - 512; // Center at 0
  
  // Apply deadzone
  if (abs(joystickX) < JOYSTICK_DEADZONE) joystickX = 0;
  if (abs(joystickY) < JOYSTICK_DEADZONE) joystickY = 0;
  
  // Map joystick values to cursor movement speed
  int moveX = map(joystickX, -512, 512, -CURSOR_SPEED, CURSOR_SPEED);
  int moveY = map(joystickY, -512, 512, -CURSOR_SPEED, CURSOR_SPEED);
  
  // Invert Y axis so pushing forward moves cursor up
  moveY = -moveY;
  
  // Move the cursor
  if (moveX != 0 || moveY != 0) {
    Mouse.move(moveX, moveY, 0);
    
    // For debugging
    Serial.print("Cursor move: X=");
    Serial.print(moveX);
    Serial.print(", Y=");
    Serial.println(moveY);
  }
} 
