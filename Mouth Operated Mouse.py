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
        ports_to_try = ['COM1', 'COM2', 'COM3', 'COM5', '/dev/ttyUSB0', '/dev/ttyACM0']
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
                y_move = int(y * sensitivity)
                
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
            else:
                print("discarded left click")
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
            else:
                print("discarded right click")
                
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
