// ENGR40M_lab3b - Maze Game Arduino Sketch

// Define the LED matrix (conceptual - adapt to your hardware)
// For a real 8x8 matrix, you'd use a library like LedControl.h or direct pin manipulation.
// This sketch focuses on the logic.

// Maze dimensions
const int MAZE_WIDTH = 8;
const int MAZE_HEIGHT = 8;

// Maze layout
// 0: Path, 1: Wall, 2: Start, 3: End
char maze[MAZE_HEIGHT][MAZE_WIDTH] = {
  {'1', '1', '1', '1', '1', '1', '1', '1'},
  {'1', '2', '0', '0', '0', '0', '0', '1'},
  {'1', '1', '1', '0', '1', '1', '0', '1'},
  {'1', '0', '0', '0', '0', '1', '0', '1'},
  {'1', '0', '1', '1', '1', '1', '0', '1'},
  {'1', '0', '0', '0', '0', '0', '3', '1'},
  {'1', '1', '1', '1', '1', '1', '1', '1'},
  {'1', '1', '1', '1', '1', '1', '1', '1'}
};

// Player position
int playerX = 1;
int playerY = 1; // Initial position (corresponds to '2')

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Arduino Maze Game Ready!");
  Serial.println("Send U, D, L, R for Up, Down, Left, Right.");

  // Find starting position if not hardcoded
  for (int y = 0; y < MAZE_HEIGHT; y++) {
    for (int x = 0; x < MAZE_WIDTH; x++) {
      if (maze[y][x] == '2') {
        playerX = x;
        playerY = y;
        break;
      }
    }
  }
  
  displayPlayer(); // Initial display
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    movePlayer(command);
  }
}

void movePlayer(char direction) {
  int newX = playerX;
  int newY = playerY;

  switch (direction) {
    case 'U': // Up
      newY--;
      break;
    case 'D': // Down
      newY++;
      break;
    case 'L': // Left
      newX--;
      break;
    case 'R': // Right
      newX++;
      break;
    default:
      Serial.println("Invalid command");
      return;
  }

  // Check boundaries and walls
  if (newX >= 0 && newX < MAZE_WIDTH && newY >= 0 && newY < MAZE_HEIGHT && maze[newY][newX] != '1') {
    playerX = newX;
    playerY = newY;
    Serial.print("Moved to: "); Serial.print(playerX); Serial.print(","); Serial.println(playerY);
    displayPlayer();

    if (maze[playerY][playerX] == '3') {
      Serial.println("Congratulations! You reached the end!");
      // Optional: Reset game or stop
    }
  } else {
    Serial.println("Move blocked: Wall or boundary.");
  }
}

// Placeholder for displaying the player on the LED matrix
void displayPlayer() {
  // Clear the matrix (conceptual)
  // clearLedMatrix(); 

  // Light up the LED at (playerX, playerY) (conceptual)
  // setLed(playerX, playerY, HIGH);

  Serial.println("---- LED Matrix Display ----");
  for (int y = 0; y < MAZE_HEIGHT; y++) {
    String line = "";
    for (int x = 0; x < MAZE_WIDTH; x++) {
      if (x == playerX && y == playerY) {
        line += "P "; // Player
      } else if (maze[y][x] == '1') {
        line += "# "; // Wall
      } else if (maze[y][x] == '3') {
        line += "E "; // End
      } else {
        line += ". "; // Path
      }
    }
    Serial.println(line);
  }
  Serial.println("--------------------------");
}

// Placeholder function to clear LED matrix (implement based on your hardware)
// void clearLedMatrix() { /* ... your code ... */ }

// Placeholder function to set an LED (implement based on your hardware)
// void setLed(int x, int y, bool state) { /* ... your code ... */ }
