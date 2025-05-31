from fastapi import FastAPI, HTTPException
from fastapi.staticfiles import StaticFiles
from fastapi.responses import JSONResponse, FileResponse
from pydantic import BaseModel
import serial
import serial.tools.list_ports
import time
import random
import os
import json
import argparse
from pathlib import Path

# Parse command line arguments
parser = argparse.ArgumentParser(description='Maze Game Backend')
parser.add_argument('--port', type=int, default=3002, help='Web server port')
parser.add_argument('--arduino-port', type=str, default=None, help='Arduino serial port')
parser.add_argument('--debug', action='store_true', help='Run in debug mode without Arduino')
args = parser.parse_args()

app = FastAPI()
SERIAL_PORT = args.arduino_port  # Will be auto-detected if None
BAUD_RATE = 9600
ser = None
DEBUG_MODE = args.debug

# Set up paths
BASE_DIR = Path(__file__).resolve().parent.parent
frontend_dir = os.path.join(BASE_DIR, "frontend")

# Serve static files
app.mount("/static", StaticFiles(directory=frontend_dir), name="static")

# Game state
maze = []
player_pos = (1, 1)
end_pos = (6, 6)

def is_valid(x, y, width, height):
    return 0 <= x < width and 0 <= y < height

def generate_maze_prims():
    """Generate a maze using Prim's algorithm"""
    width, height = 8, 8
    # Initialize maze with walls (1)
    maze = [[1 for _ in range(width)] for _ in range(height)]
    
    # Start at a random cell and mark it as part of the maze
    start_x, start_y = 1, 1  # Start position is fixed
    end_x, end_y = width-2, height-2  # End position in bottom right
    
    # Mark the start cell as path
    maze[start_y][start_x] = 0
    
    # Add the walls of the start cell to the wall list
    walls = []
    # Check neighboring cells (up, right, down, left)
    directions = [(0, 1), (1, 0), (0, -1), (-1, 0)]
    
    # Add walls around the start cell
    for dx, dy in directions:
        nx, ny = start_x + dx, start_y + dy
        if is_valid(nx, ny, width, height):
            walls.append((nx, ny))
    
    # While there are walls in the list
    while walls:
        # Pick a random wall
        wall_index = random.randint(0, len(walls) - 1)
        wall_x, wall_y = walls.pop(wall_index)
        
        # Count path cells around this wall
        path_count = 0
        path_cells = []
        
        for dx, dy in directions:
            nx, ny = wall_x + dx, wall_y + dy
            if is_valid(nx, ny, width, height) and maze[ny][nx] == 0:
                path_count += 1
                path_cells.append((nx, ny))
        
        # If exactly one neighboring cell is a path, make this wall a path
        if path_count == 1:
            maze[wall_y][wall_x] = 0
            
            # Add neighboring walls to the wall list
            for dx, dy in directions:
                nx, ny = wall_x + dx, wall_y + dy
                if is_valid(nx, ny, width, height) and maze[ny][nx] == 1:
                    if (nx, ny) not in walls:
                        walls.append((nx, ny))
    
    # Ensure start and end are open
    maze[start_y][start_x] = 0  # Start position (player)
    maze[end_y][end_x] = 2      # End position (goal)
    
    # Make sure there's a path to the exit
    # Simple approach: clear a path if needed
    for i in range(start_x, end_x + 1):
        if random.random() < 0.7:  # 70% chance to clear
            maze[start_y][i] = 0
    for j in range(start_y, end_y + 1):
        if random.random() < 0.7:  # 70% chance to clear
            maze[j][end_x] = 0
    
    return maze

def find_arduino_port():
    """Try to auto-detect the Arduino port"""
    print("Looking for Arduino ports...")
    ports = list(serial.tools.list_ports.comports())
    
    for port in ports:
        if 'Arduino' in port.description or 'usbmodem' in port.device or 'cu.' in port.device:
            print(f"Found potential Arduino at {port.device} - {port.description}")
            return port.device
    
    # Common port names on different platforms
    common_ports = [
        '/dev/ttyACM0',  # Linux
        '/dev/ttyUSB0',  # Linux
        '/dev/cu.usbmodem1401',  # Mac
        '/dev/cu.usbmodem1301',  # Mac
        '/dev/cu.usbmodem14201',  # Mac
        'COM3',  # Windows
        'COM4',  # Windows
    ]
    
    # Print all found ports for debugging
    print("All available ports:")
    for port in ports:
        print(f"  {port.device} - {port.description}")
    
    return None

def connect_arduino():
    """Connect to the Arduino via serial port"""
    global ser, SERIAL_PORT
    
    if DEBUG_MODE:
        print("Running in debug mode - skipping Arduino connection")
        return True
    
    # Auto-detect port if not specified
    if SERIAL_PORT is None:
        SERIAL_PORT = find_arduino_port()
        if SERIAL_PORT is None:
            print("Could not auto-detect Arduino port. Available ports:")
            ports = list(serial.tools.list_ports.comports())
            for port in ports:
                print(f"  {port.device} - {port.description}")
            print("Run with --arduino-port PORT to specify a port")
            print("Or run with --debug to use debug mode without Arduino")
            return False
    
    try:
        print(f"Connecting to Arduino on {SERIAL_PORT}...")
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Wait for Arduino to initialize
        response = send_command("PING")
        if "PONG" in response:
            print("Arduino connected successfully")
            return True
        else:
            print(f"Arduino not responding properly: {response}")
            return False
    except Exception as e:
        print(f"Failed to connect to Arduino: {e}")
        return False

def send_command(command):
    """Send a command to Arduino and get response"""
    global ser
    
    if DEBUG_MODE:
        print(f"DEBUG: Would send to Arduino: {command}")
        if command == "PING":
            return "PONG"
        return "OK"
    
    if ser is None:
        print("Arduino not connected")
        return ""
    
    try:
        ser.write(f"{command}\n".encode())
        time.sleep(0.1)  # Give Arduino time to process
        response = ""
        while ser.in_waiting:
            response += ser.readline().decode().strip() + "\n"
        return response
    except Exception as e:
        print(f"Error sending command to Arduino: {e}")
        return ""

def send_maze_to_arduino():
    """Send the maze data to Arduino"""
    global maze, player_pos
    
    if DEBUG_MODE:
        print("DEBUG: Would send maze to Arduino")
        return True
        
    if ser is None and not DEBUG_MODE:
        print("Arduino not connected - skipping maze send")
        return False
    
    try:
        # Wait a moment to ensure Arduino is ready
        time.sleep(0.5)
        
        # Format maze data in a simpler format for Arduino
        maze_data = json.dumps(maze)
        print(f"Sending maze data: {maze_data}")
        response = send_command(f"MAZE:{maze_data}")
        print(f"Arduino response: {response}")
        
        # Wait a moment before sending player position
        time.sleep(0.5)
        
        # Send player position
        x, y = player_pos
        print(f"Sending player position: {x},{y}")
        response = send_command(f"PLAYER:{x},{y}")
        print(f"Arduino response: {response}")
        
        return True
    except Exception as e:
        print(f"Error sending maze to Arduino: {e}")
        return False

@app.on_event("startup")
def startup_event():
    global maze, player_pos, end_pos
    maze = generate_maze_prims()
    player_pos = (1, 1)
    end_pos = (6, 6)
    connect_arduino()
    send_maze_to_arduino()

class MoveRequest(BaseModel):
    direction: str

@app.get("/move/{direction}")
def move_player(direction: str):
    global player_pos, maze
    x, y = player_pos
    new_x, new_y = x, y
    
    if direction == "up" and y > 0:
        new_y = y - 1
    elif direction == "down" and y < 7:
        new_y = y + 1
    elif direction == "left" and x > 0:
        new_x = x - 1
    elif direction == "right" and x < 7:
        new_x = x + 1
    
    # Debug output
    print(f"Attempting to move from ({x},{y}) to ({new_x},{new_y})")
    print(f"Cell value at destination: {maze[new_y][new_x]}")
    
    # Check if the new position is a valid path (0) or the exit (2)
    if maze[new_y][new_x] != 1:  # Not a wall
        player_pos = (new_x, new_y)
        print(f"Movement allowed. Player now at: {player_pos}")
        
        # Send updated position to Arduino ONLY if move is valid
        if ser is not None:
            print(f"Sending player position to Arduino: {player_pos}")
            send_command(f"PLAYER:{player_pos[0]},{player_pos[1]}")
        
        # Check for victory
        won = maze[new_y][new_x] == 2
        if won:
            print("Player won! Sending WIN command to Arduino")
            if ser is not None:
                response = send_command("WIN:TRUE")
                print(f"Arduino WIN response: {response}")
                # Wait a moment for the victory animation
                time.sleep(1)
            return {"message": "You won!", "maze": maze, "player_pos": player_pos, "won": True}
    else:
        print(f"Move blocked by wall at {new_x},{new_y}")
        # Don't update player_pos or send to Arduino
    
    return {"maze": maze, "player_pos": player_pos, "won": False}

@app.post("/api/new_maze")
def new_maze():
    global maze, player_pos, end_pos
    
    # Send reset command to Arduino first to clear any victory state
    if ser is not None:
        print("Sending RESET command to Arduino")
        response = send_command("RESET:TRUE")
        print(f"Arduino reset response: {response}")
        time.sleep(0.5)  # Give Arduino time to reset
    
    # Generate a new maze using Prim's algorithm
    maze = generate_maze_prims()
    
    # Set player and end positions (always in opposite corners)
    player_pos = (1, 1)
    end_pos = (len(maze[0])-2, len(maze)-2)  # Bottom-right corner
    
    # Make sure start and end positions are paths
    maze[player_pos[1]][player_pos[0]] = 0  # Start is empty
    maze[end_pos[1]][end_pos[0]] = 2        # End is marked with 2
    
    # Send to Arduino if connected
    send_maze_to_arduino()
    
    return {"maze": maze, "player_pos": player_pos}

# Root endpoint to serve the main page
@app.get("/")
async def read_root():
    return FileResponse(os.path.join(frontend_dir, "test.html"))

@app.get("/api/maze")
def get_maze():
    return {"maze": maze, "player_pos": player_pos, "end_pos": end_pos}

if __name__ == "__main__":
    import uvicorn
    # Use the port from command line arguments
    uvicorn.run(app, host="0.0.0.0", port=args.port)