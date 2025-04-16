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
const int HARD_PUFF_THRESHOLD = 700; // Threshold for left click
const int PRESSURE_DELAY = 50; // number of readings to check before sending commands
const float ERROR_THRESHOLD = 0.6f; // readings must 

volatile unsigned long long sampleBufferIndex = 0;
volatile int sampleBuffer[PRESSURE_DELAY];

const int NEUTRAL = 0;
const int HARD_SIP = 1;
const int SOFT_SIP = 2;
const int HARD_PUFF = 3;
const int SOFT_PUFF = 4;

volatile int counts[5] = {PRESSURE_DELAY, 0,0,0,0};
volatile float percentages[5] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f};

// counts[CODE] is the count of our state
// e.g. counts[NEUTRAL] gives us neutral

const int UPDATE_INTERVAL = 8; // number of samples needed before updating state


void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("Arduino Mouth-Operated Mouse - Serial Version");
  
  // Initialize timers
  sampleTimer = millis() + SAMPLE_PERIOD;
  cursorTimer = millis() + CURSOR_UPDATE_PERIOD;

  //Serial.println("neutral,hardsip,softsip,hardpuff,softpuff");
}

void debugPrint(int value) {
  //Serial.print(value);
  //Serial.print(" ");
  Serial.print(percentages[0]);
  Serial.print(",");
  Serial.print(percentages[1]);
  Serial.print(",");
  Serial.print(percentages[2]);
  Serial.print(",");
  Serial.print(percentages[3]);
  Serial.print(",");
  Serial.println(percentages[4]);

}

void loop() {
  // Sample pressure sensor at regular intervals
  if (millis() >= sampleTimer) {
    // samplePressure();

    int value = analogRead(A0);
    processPressure(value);

    debugPrint(value);


    sampleTimer = millis() + SAMPLE_PERIOD;
  }
  
  // Process pressure samples when we have enough
  /*
  if (sampleCounter >= SAMPLE_LENGTH) {
    processPressure(calculateAveragePressure());
    sampleCounter = 0;
  }*/
  
  // Update cursor position at regular intervals
  if (millis() >= cursorTimer) {
    updateCursorPosition();
    cursorTimer = millis() + CURSOR_UPDATE_PERIOD;
  }
}

void addSample(int sample) {
  int sampleCode = NEUTRAL;

  // if its greater than neutral max, it must be a puff of some kind
  if (sample > NEUTRAL_MAX) {
    if (sample < SOFT_PUFF_THRESHOLD) {
      sampleCode = SOFT_PUFF;
    } else if (sample < HARD_PUFF_THRESHOLD) {
      sampleCode = HARD_PUFF;
    }
  }
  else if (sample < NEUTRAL_MIN) {
    if (sample > SOFT_SIP_THRESHOLD) {
      sampleCode = SOFT_SIP;
    } else if (sample > HARD_SIP_THRESHOLD) {
      sampleCode = HARD_SIP;
    }
  }

  sampleBufferIndex++; // TODO overflow

  int lastSampleCode = sampleBuffer[sampleBufferIndex % PRESSURE_DELAY];
  counts[lastSampleCode]--; // remove overwritten count
  counts[sampleCode]++; // add new count
  sampleBuffer[sampleBufferIndex % PRESSURE_DELAY] = sampleCode;

}

void updatePercentages() {
  for (int i = 0; i < 5; i++) {
    percentages[i] = (float)counts[i] / (float)PRESSURE_DELAY;
  }
}

int chooseState(int prevState) {
  int newState = prevState;
  float bestThreshold = 0.0f;

  for (int i = 0; i < 5; i++) {
    if (percentages[i] >= ERROR_THRESHOLD && percentages[i] > bestThreshold) {
      newState = i;
      bestThreshold = percentages[i];
    }
  }
  return newState;
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
/*
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
*/


void processPressure(int pressure) {
  int lastState = sampleBuffer[sampleBufferIndex % PRESSURE_DELAY];

  addSample(pressure);

  if (sampleBufferIndex % UPDATE_INTERVAL == 0) {
    updatePercentages();
    int newState = chooseState(lastState);


    const char* commands[5] = {"NEUTRAL", "RIGHT_CLICK_DOWN", "SCROLL_DOWN", "LEFT_CLICK_DOWN", "SCROLL_UP"};

    
    //Serial.println(commands[newState]);
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
