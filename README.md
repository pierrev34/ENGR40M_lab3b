# Arduino LED Matrix Maze Game

A simple maze game running on an Arduino with an 8x8 LED matrix display. The game features a player that can be moved around the maze to reach the exit.

## Project Structure

```
ENGR40M_lab3b/
├── arduino/                    # Arduino sketch files
│   └── matrix_basics/         
│       └── matrix_basics.ino  # Main Arduino sketch
├── standalone_maze.html       # Web interface for game control
└── README.md                  # This file
```

## Hardware Requirements

- Arduino board (Uno, Nano, etc.)
- 8x8 LED matrix with common cathode
- Jumper wires
- 220Ω resistors (8x)
- Breadboard

## Pin Configuration

### LED Matrix Connections:
- Rows (anodes): Connect to Arduino pins 2-9
- Columns (cathodes): Connect to Arduino pins 10-17

## Setup Instructions

### 1. Upload the Sketch
1. Open `arduino/matrix_basics/matrix_basics.ino` in Arduino IDE
2. Select your board and port from the Tools menu
3. Click the Upload button
4. Open the Serial Monitor (baud rate 9600) to see debug messages

### 2. Web Interface (Optional)
1. Open `standalone_maze.html` in a web browser
2. Use the arrow keys or on-screen buttons to control the player

## How to Play
- Use arrow keys to move the player (red LED)
- Reach the exit (blinking green LED)
- Avoid walls (unlit LEDs)

## Troubleshooting
- If the display is flickering, try adjusting the delay in the `loop()` function
- Ensure all connections are secure and correct
- Check the Serial Monitor for error messages
- Make sure the LED matrix is properly connected with correct polarity

## License

This project is open source and available under the [MIT License](LICENSE).

## Contributing

Feel free to fork this repository and submit pull requests. For major changes, please open an issue first to discuss what you would like to change.
