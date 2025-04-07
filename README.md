# Arduino Mouth-Operated Mouse

An open-source, affordable assistive technology project that enables computer control through mouth movements and sip/puff actions. This device is similar to commercial solutions like QuadJoy or LipStick but at a fraction of the cost. This project aims to reduce the cost to access computers for people with physical disabilities, since official commerical products can have prices of up to 2000 USD.  
Know that this version is made for the Arduino Uno, and runs through a python code
## Overview

This project creates a mouth-operated mouse using an Arduino Uno that combines a pressure sensor for sip/puff actions with a joystick for cursor movement, allowing users with limited hand mobility to control a computer mouse. The device functions as follows:

- **Joystick**: Controls cursor movement (operated by mouth/chin)
- **Sip/Puff Actions**:
  - Hard sip → Right click
  - Soft sip → Scroll down
  - Neutral → No action
  - Soft puff → Scroll up
  - Hard puff → Left click

## Features

- Low-cost alternative to commercial assistive mouse devices
- Customizable sensitivity and thresholds
- Arduino Uno + Python application implementation
- Open-source hardware and software
- Modular design for easy customization

## Hardware Requirements

### Essential Components

1. **Arduino Uno** (~$25 USD)
   - Core controller for the project

2. **Pressure Sensor** - Options include:
   - MPXV7002DP (~$20 USD) - Specifically designed for sip/puff applications
   - Adafruit MPRLS Ported Pressure Sensor Breakout (~$15 USD)
   - MPX5010DP (~$10-15 USD) - More affordable alternative

3. **Joystick Module**
   - Analog Thumb Joystick Module (~$5-10 USD)

4. **Tubing and Mouthpiece:**
   - Food-grade silicone tubing (~$5 USD) - 1/8" inner diameter
   - Plastic mouthpiece (custom-made or adapted)

5. **Additional Components:**
   - Breadboard for prototyping (~$5 USD)
   - Jumper wires (~$3-5 USD)
   - USB cable for Arduino (~$3-5 USD)
   - Small project box/enclosure (~$5-10 USD)

**Estimated Total Cost:** Approximately $60-80 USD


## Circuit Diagram

### Basic Circuit Connections

1. **Pressure Sensor (MPXV7002DP or similar) Connections:**
   - GND pin → Arduino GND
   - +5V pin → Arduino 5V
   - Analog output pin → Arduino A0

2. **Joystick Module Connections:**
   - GND pin → Arduino GND
   - +5V pin → Arduino 5V
   - VRx (X-axis) → Arduino A1
   - VRy (Y-axis) → Arduino A2
   - SW (Switch, optional) → Arduino digital pin 2

3. **Tubing Connection:**
   - Connect silicon food-grade tubing to the pressure port on the sensor
   - The other end of the tubing connects to a mouthpiece

## Software

### Arduino Code

```cpp
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

```

### Python Application

```python
import serial
import time
import autopy
import pyautogui 

# Change 'COM4' to your Arduino's port (on Windows)
try:
    # Try to open the serial port, with error handling
    try:
        ser = serial.Serial('COM4', 115200, timeout=0.1)
        print(f"Connected to COM4")
    except serial.SerialException as e:
        # If COM4 fails, try common alternatives
        ports_to_try = ['COM3', 'COM5', '/dev/ttyUSB0', '/dev/ttyACM0']
        for port in ports_to_try:
            try:
                print(f"Trying to connect to {port}...")
                ser = serial.Serial(port, 115200, timeout=0.1)
                print(f"Connected to {port}")
                break
            except serial.SerialException:
                continue
        else:  # This else belongs to the for loop, executes if no break occurred
            raise Exception("Could not connect to any serial port. Please check your connections and specify the correct port.")
except Exception as e:
    print(f"Error: {e}")
    print("Please update the script with the correct port and try again.")
    import sys
    sys.exit(1)

sensitivity = 0.03  # Adjust this value to change cursor movement sensitivity

scroll_strength = 10  # Number of scroll units per command

left_click_active = False
right_click_active = False
last_left_click_time = 0
last_right_click_time = 0
click_cooldown = 1  # Seconds between clicks to prevent rapid clicking

def process_command(command):
    global left_click_active, right_click_active, last_left_click_time, last_right_click_time
    current_time = time.time()
    
    if command.startswith("MOVE"):
        parts = command.split(',')
        if len(parts) == 3:
            try:
                x = int(parts[1])
                y = int(parts[2])
                
                x_move = int(x * sensitivity)
                y_move = int(-y * sensitivity)
                
                current_x, current_y = autopy.mouse.location()
                
                new_x = current_x + x_move
                new_y = current_y - y_move  
                
                screen_width, screen_height = autopy.screen.size()
                new_x = max(0, min(new_x, screen_width - 1))
                new_y = max(0, min(new_y, screen_height - 1))
                
                autopy.mouse.move(new_x, new_y)
            except ValueError:
                print(f"Invalid movement values: {command}")
            except Exception as e:
                print(f"Error moving mouse: {e}")
    
    elif command == "LEFT_CLICK_DOWN":
        try:
            # Hard puff → Left click
            # Only perform click if not already active and cooldown period has passed
            if not left_click_active and (current_time - last_left_click_time) > click_cooldown:
                autopy.mouse.click()  # Default is left click
                left_click_active = True
                last_left_click_time = current_time
        except Exception as e:
            print(f"Error with left click: {e}")
    
    elif command == "RIGHT_CLICK_DOWN":
        try:
            # Hard sip → Right click
            # Only perform click if not already active and cooldown period has passed
            if not right_click_active and (current_time - last_right_click_time) > click_cooldown:
                autopy.mouse.click(autopy.mouse.Button.RIGHT)
                right_click_active = True
                last_right_click_time = current_time
        except Exception as e:
            print(f"Error with right click: {e}")
    
    elif command == "NEUTRAL":
        # Reset button states
        left_click_active = False
        right_click_active = False
    
    elif command == "SCROLL_UP":
        try:
            # Soft puff → Scroll up
            # Using PyAutoGUI for scrolling with increased strength
            pyautogui.scroll(scroll_strength)  
        except Exception as e:
            print(f"Error scrolling up: {e}")
    
    elif command == "SCROLL_DOWN":
        try:
            # Soft sip → Scroll down
            pyautogui.scroll(-scroll_strength)  
        except Exception as e:
            print(f"Error scrolling down: {e}")
            

print("This implementation uses:")
print("- AutoPy for mouse movement and clicking")
print("- PyAutoGUI for scrolling functionality with increased strength")
print(f"- Scroll strength set to {scroll_strength}")
print("Press Ctrl+C to exit.")

try:
    try:
        screen_width, screen_height = autopy.screen.size()
        print(f"Screen size: {screen_width}*{screen_height}")
        print(f"Initial mouse position: {autopy.mouse.location()}")
    except Exception as e:
        print(f"Warning: Could not get screen information: {e}")
        print("This may indicate that AutoPy cannot access the display.")
        print("Make sure you're running this in a graphical environment.")
    
    # Test scrolling functionality
    try:
        print("Testing scroll functionality...")
        print("Scroll up test:")
        pyautogui.scroll(1)
        print("Scroll down test:")
        pyautogui.scroll(-1)
        print("Scroll test completed successfully")
    except Exception as e:
        print(f"Warning: Scroll test failed: {e}")
        print("Scrolling functionality may not work properly")
    
    # Main loop
    while True:
        if ser.in_waiting > 0:
            try:
                line = ser.readline().decode('utf-8').strip()
                if line:
                    print(f"Received: {line}")
                    process_command(line)
            except UnicodeDecodeError:
                print("Warning: Received invalid data from serial port")
            except Exception as e:
                print(f"Error processing serial data: {e}")
        
        time.sleep(0.01)  # Small delay to prevent CPU hogging
        
except KeyboardInterrupt:
    print("\nProgram terminated by user")
except Exception as e:
    print(f"\nUnexpected error: {e}")
finally:
    # Clean up
    try:
        # Close serial connection
        ser.close()
        print("Serial connection closed")
    except:
        pass
    
    print("Program ended")

```

### Python Setup Instructions:

1. **Install required Python packages:**
   ```
   pip install pyserial pyautogui
   pip install -U autopy
   ```



2. **Save the code** as `mouse_controller.py` if you are not using any IDE

3. **Run the script** from command line:
   ```
   python mouse_controller.py
   ```

## Assembly Instructions

### Step 1: Prepare Your Components
1. Ensure your Arduino Uno is functioning properly by connecting it to your computer and uploading a simple test sketch (like Blink).
2. Gather all components.
3. Install required Python packages on your computer.

### Step 2: Connect the Pressure Sensor
1. Place the MPXV7002DP pressure sensor on your breadboard.
2. Make the following connections:
   - Connect GND pin on the sensor to GND on the Arduino
   - Connect +5V pin on the sensor to 5V on the Arduino
   - Connect the analog output pin on the sensor to A0 on the Arduino

### Step 3: Connect the Joystick Module
1. Place the analog joystick module on your breadboard.
2. Make the following connections:
   - Connect GND on the joystick to GND on the Arduino
   - Connect +5V on the joystick to 5V on the Arduino
   - Connect VRx (X-axis) on the joystick to A1 on the Arduino
   - Connect VRy (Y-axis) on the joystick to A2 on the Arduino
   - Connect SW (switch, if using) on the joystick to digital pin 2 on the Arduino

### Step 4: Prepare the Mouthpiece and Tubing
1. Cut a piece of food-grade silicone tubing to your desired length (approximately 2-3 feet).
2. If using a custom mouthpiece:
   - Clean the mouthpiece thoroughly
   - Ensure it fits comfortably in your mouth
   - Connect one end of the tubing to the mouthpiece, securing with hot glue if necessary
3. If not using a custom mouthpiece:
   - You can create a simple mouthpiece by slightly flaring one end of the tubing
   - Alternatively, adapt a small plastic tube or straw holder

### Step 5: Connect the Tubing to the Pressure Sensor
1. Connect the free end of the silicone tubing to the pressure port on the MPXV7002DP sensor.
2. Ensure the connection is airtight - you may need to use a small amount of hot glue to secure it.
3. If the tubing is too large for the pressure port, you can use a smaller diameter tube as an adapter.

### Step 6: Upload the Arduino Code
1. Connect your Arduino to your computer via USB.
2. Open the Arduino IDE.
3. Create a new sketch and paste the Arduino code provided above or just download the code provided.
4. Select the correct board (Arduino Uno) and port.
5. Upload the code to your Arduino.

### Step 7: Set Up the Python Application
1. Create a new Python file named `mouse_controller.py` and paste the Python code provided above or just download the code provided.
2. Update the serial port in the code to match your Arduino's port.
3. Run the Python script to start controlling your mouse.

## Testing and Calibration

1. Open the Serial Monitor in the Arduino IDE
2. Test the pressure sensor by gently sipping and puffing on the mouthpiece
3. Observe the pressure readings and adjust the threshold values in the code if necessary
4. Test the joystick by moving it in different directions
5. Make adjustments to sensitivity and deadzone values as needed

## Mouthpiece Design Options

Several options are available for the mouthpiece:

1. **Simple Tube End**: The simplest option is to use the silicone tube itself, possibly flared slightly at the end for comfort.

2. **Adapted Straw Holder**: A plastic straw holder or small mouthpiece from another device can be adapted.

3. **3D Printed Custom Mouthpiece**: For those with access to a 3D printer, a custom mouthpiece can be designed and printed.

4. **Modified Medical Mouthpiece**: Repurposing a mouthpiece from a medical device like a spirometer.


## Acknowledgments

- Based on "The 'Sup" open-source project by Jacob Field
  - Project page: [The 'Sup - a Mouse for People With Quadriplegia](https://www.instructables.com/The-Sup-a-Mouse-for-Quadriplegics-Low-Cost-and-Ope/)
  - GitHub repository: [https://github.com/Bobcatmodder/SipNPuff_Mouse](https://github.com/Bobcatmodder/SipNPuff_Mouse)
- Inspired by commercial assistive technology devices like QuadJoy and LipStick
- Thanks to the Arduino and open-source hardware communities

## Limitations/future implementations

- Drag click/Drag and drop  
- Keyboard access  
- Voice to text  
- Keyboard shortcuts  
- Middle click  
