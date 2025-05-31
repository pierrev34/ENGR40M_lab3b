# Maze Game - Arduino, Python (FastAPI), Web Frontend

This project is a simple maze game where the player is controlled via a web interface.
The movement commands are sent from the web frontend to a Python FastAPI backend, which then relays them to an Arduino over serial communication. The Arduino updates the player's position in the maze and (conceptually) displays it on an 8x8 LED matrix.

## Project Structure

```
ENGR40M_lab3b/
├── arduino_maze_game/
│   └── arduino_maze_game.ino  # Arduino sketch
├── python_backend/
│   ├── main.py                # FastAPI backend server
│   └── requirements.txt       # Python dependencies
├── frontend/
│   ├── index.html             # HTML for the game interface
│   ├── script.js              # JavaScript for frontend logic
│   └── style.css              # CSS for styling
└── README.md                  # This file
```

## Setup and Running

### 1. Arduino Setup

1.  **Hardware**:
    *   Arduino board (e.g., Uno, Nano, etc.)
    *   (Optional but intended) 8x8 LED Matrix and necessary driving circuitry. The current `.ino` file has placeholder functions for LED control (`displayPlayer`, `clearLedMatrix`, `setLed`). You will need to implement these based on your specific LED matrix and how it's wired (e.g., using MAX7219, shift registers, or direct pin control).
2.  **Software**:
    *   Open `arduino_maze_game/arduino_maze_game.ino` in the Arduino IDE.
    *   Select your board and port from the `Tools` menu.
    *   Upload the sketch to your Arduino.
    *   Open the Serial Monitor (set baud rate to 9600). You should see "Arduino Maze Game Ready!".

### 2. Python Backend Setup

1.  **Navigate to the project root directory** in your terminal:
    ```bash
    cd /Users/pierre/Desktop/ENGR40M_lab3b
    ```
2.  **Create a virtual environment (recommended)**:
    ```bash
    python3 -m venv venv
    source venv/bin/activate  # On Windows: venv\Scripts\activate
    ```
3.  **Install dependencies**:
    ```bash
    pip install -r python_backend/requirements.txt
    ```
4.  **Configure Serial Port**: 
    *   Open `python_backend/main.py`.
    *   **CRITICAL**: Modify the `SERIAL_PORT` variable to match your Arduino's serial port. 
        *   On macOS, it might be like `/dev/tty.usbmodemXXXX` or `/dev/tty.wchusbserialXXXX`.
        *   On Linux, it might be like `/dev/ttyACM0` or `/dev/ttyUSB0`.
        *   On Windows, it might be like `COM3`, `COM4`, etc.
    *   You can find the port name in the Arduino IDE (Tools > Port) after connecting your Arduino.
5.  **Run the backend server**:
    From the `ENGR40M_lab3b` directory (the project root):
    ```bash
    uvicorn python_backend.main:app --reload --host 0.0.0.0 --port 8000
    ```
    You should see output indicating the server is running, e.g., `Uvicorn running on http://0.0.0.0:8000`. The backend will attempt to connect to the Arduino on startup.

### 3. Frontend (Web Interface)

1.  Once the Python backend is running, open your web browser.
2.  Navigate to `http://127.0.0.1:8000` or `http://localhost:8000`.
3.  You should see the maze game interface.
4.  Use the arrow keys on your keyboard to move the player ('P').

## How it Works

1.  **Frontend (`index.html`, `script.js`, `style.css`)**:
    *   Displays the maze.
    *   Captures arrow key presses.
    *   Sends a POST request to the backend's `/move` endpoint with the direction.
    *   Optimistically updates the player's position on the frontend.
2.  **Backend (`python_backend/main.py`)**:
    *   Receives the move command from the frontend.
    *   Translates the command (e.g., "ArrowUp" to 'U').
    *   Sends the single character command ('U', 'D', 'L', 'R') to the Arduino via the configured serial port using `pyserial`.
3.  **Arduino (`arduino_maze_game.ino`)**:
    *   Listens for serial commands.
    *   When a command is received, it attempts to move the player.
    *   Checks for wall collisions and boundaries based on its internal maze representation.
    *   If the move is valid, updates the player's coordinates.
    *   Calls `displayPlayer()` which (conceptually) updates the 8x8 LED matrix. This function currently prints the maze and player position to the Serial Monitor for debugging if you don't have an LED matrix set up.
    *   Sends status messages back over serial (e.g., "Moved to: X,Y", "Move blocked"), which can be seen in the Arduino Serial Monitor or (optionally) read by the Python backend if extended.

## Customization

*   **Maze Design**: Modify the `maze` array in both `arduino_maze_game.ino` and `frontend/script.js` to change the maze layout. Ensure they are consistent.
*   **LED Matrix Control**: Implement the actual hardware control for your 8x8 LED matrix in the `displayPlayer()`, `clearLedMatrix()`, and `setLed()` functions in the `.ino` file.
*   **Serial Port**: Double-check and update `SERIAL_PORT` in `python_backend/main.py`.
*   **Styling**: Modify `frontend/style.css` to change the appearance of the web interface.

Enjoy your maze game project!
