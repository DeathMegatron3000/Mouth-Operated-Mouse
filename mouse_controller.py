import serial
import time
import pyautogui

# Configure the serial connection
# Change 'COM3' to your Arduino's port (on Windows)
# On Mac/Linux, use something like '/dev/ttyUSB0' or '/dev/ttyACM0'
ser = serial.Serial('COM3', 9600, timeout=0.1)

# Set up pyautogui
pyautogui.FAILSAFE = True  # Move mouse to corner to abort
sensitivity = 0.1  # Adjust this value to change cursor movement sensitivity

def process_command(command):
    if command.startswith("MOVE"):
        # Format: MOVE,x,y
        parts = command.split(',')
        if len(parts) == 3:
            try:
                x = int(parts[1])
                y = int(parts[2])
                
                # Scale the values for smoother movement
                x_move = int(x * sensitivity)
                y_move = int(y * sensitivity)
                
                # Move mouse relative to current position
                pyautogui.moveRel(x_move, -y_move)  # Invert Y for natural movement
            except ValueError:
                print(f"Invalid movement values: {command}")
    
    elif command == "LEFT_CLICK_DOWN":
        pyautogui.mouseDown(button='left')
    
    elif command == "RIGHT_CLICK_DOWN":
        pyautogui.mouseDown(button='right')
    
    elif command == "NEUTRAL":
        # Release any pressed buttons
        pyautogui.mouseUp(button='left')
        pyautogui.mouseUp(button='right')
    
    elif command == "SCROLL_UP":
        pyautogui.scroll(1)  # Positive value scrolls up
    
    elif command == "SCROLL_DOWN":
        pyautogui.scroll(-1)  # Negative value scrolls down

print("Starting mouse control. Press Ctrl+C to exit.")
print("Move mouse to screen corner to abort (pyautogui failsafe)")

try:
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            if line:
                print(f"Received: {line}")
                process_command(line)
        time.sleep(0.01)  # Small delay to prevent CPU hogging
        
except KeyboardInterrupt:
    print("Program terminated by user")
finally:
    ser.close()
    print("Serial connection closed")
