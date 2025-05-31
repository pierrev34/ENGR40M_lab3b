# LED Maze Game Project

An interactive maze game with an Arduino LED matrix display and web interface.

<<<<<<< HEAD
=======
## Project Structure

```
LED_Maze_Project/
├── arduino/                 # Arduino sketch files
│   └── maze_game_arduino/   
│       └── maze_game_arduino.ino  # Main Arduino sketch
├── backend/                 # Python backend server
│   └── main.py             # FastAPI server code
└── frontend/               # Web interface
    ├── standalone_maze.html
    ├── script.js
    └── style.css
```

## Hardware Requirements

- Arduino board (Uno, Nano, etc.)
- 8x8 LED matrix with common cathode
- Jumper wires
- 220Ω resistors (8x)
- Breadboard

## Setup Instructions

### 1. Arduino Setup
1. Open `arduino/maze_game_arduino/maze_game_arduino.ino` in Arduino IDE
2. Connect your Arduino and select the correct board/port
3. Upload the sketch
4. Open Serial Monitor (baud rate 9600) for debug messages

### 2. Python Backend Setup
1. Install requirements:
   ```bash
   pip install -r backend/requirements.txt
   ```
2. Update the serial port in `backend/main.py` if needed
3. Run the server:
   ```bash
   python backend/main.py
   ```

### 3. Web Interface
1. Open `frontend/standalone_maze.html` in a web browser
2. Use arrow keys or on-screen buttons to control the game

## How to Play
- Use arrow keys to move the player (red LED)
- Reach the exit (blinking green LED)
- Avoid walls (unlit LEDs)

## Troubleshooting
- If the display flickers, adjust the delay in the Arduino's `loop()`
- Ensure all connections are secure
- Check Serial Monitor for error messages
- Verify LED matrix polarity and connections
>>>>>>> 2267e7d (Restructure project: Keep essential files and update Arduino code)
