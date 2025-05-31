/*
 * Simple Maze Game with Web Communication
 * For ENGR40M Lab 3B
 */

// Pin definitions for LED Matrix
const byte ROW_PINS[8] = {6, 7, 8, 9, 10, 11, 12, 13};  // Row pins
const byte COL_PINS[8] = {A3, A2, A1, A0, 5, 4, 3, 2};  // Column pins

// Game variables
int playerX = 1;
int playerY = 1;
int exitX = 6;
int exitY = 6;
boolean playerVisible = true;
unsigned long lastBlink = 0;

// Simple maze layout (0 = wall/off, 1 = path/on)
byte maze[8][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},  // Top border
  {0, 1, 1, 1, 1, 1, 1, 0},  // Paths are 1 (ON)
  {0, 1, 0, 0, 1, 0, 1, 0},  // Walls are 0 (OFF)
  {0, 1, 1, 1, 1, 1, 1, 0},
  {0, 1, 0, 0, 1, 0, 1, 0},
  {0, 1, 1, 1, 1, 0, 1, 0},
  {0, 1, 1, 1, 1, 1, 1, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}   // Bottom border
};

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Configure all pins
  for (int i = 0; i < 8; i++) {
    pinMode(ROW_PINS[i], OUTPUT);
    pinMode(COL_PINS[i], OUTPUT);
    digitalWrite(ROW_PINS[i], HIGH);  // Rows off initially
    digitalWrite(COL_PINS[i], HIGH);  // Columns off initially
  }
  
  // Let things settle
  delay(500);
  
  // Send ready message to web app
  Serial.println("READY:MAZE_CONTROLLER");
}

void loop() {
  // Handle player blinking
  handlePlayerBlink();
  
  // Check for serial commands
  checkSerialCommands();
  
  // Display the maze
  displayMaze();
}

// Handle the blinking of player LED
void handlePlayerBlink() {
  if (millis() - lastBlink > 300) {
    playerVisible = !playerVisible;
    lastBlink = millis();
  }
}

// Display the maze using the working row-by-row approach
void displayMaze() {
  // Scan through each row
  for (int row = 0; row < 8; row++) {
    // Turn off all rows first
    for (int r = 0; r < 8; r++) {
      digitalWrite(ROW_PINS[r], HIGH);
    }
    
    // Set the LED states for this row
    for (int col = 0; col < 8; col++) {
      if (row == playerY && col == playerX && playerVisible) {
        // Player position (blinking)
        digitalWrite(COL_PINS[col], LOW);  // ON
      } 
      else if (row == exitY && col == exitX) {
        // Exit position
        digitalWrite(COL_PINS[col], LOW);  // ON
      }
      else if (maze[row][col] == 1) {
        // Path
        digitalWrite(COL_PINS[col], LOW);  // ON
      }
      else {
        // Wall
        digitalWrite(COL_PINS[col], HIGH); // OFF
      }
    }
    
    // Enable this row
    digitalWrite(ROW_PINS[row], LOW);
    
    // Hold long enough for visibility
    delay(1);
    
    // Disable row and reset columns before moving to next row
    digitalWrite(ROW_PINS[row], HIGH);
    for (int col = 0; col < 8; col++) {
      digitalWrite(COL_PINS[col], HIGH);  // All LEDs off
    }
  }
}

// Process serial commands from web app
void checkSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    Serial.print("RECEIVED:");
    Serial.println(command);
    
    if (command.startsWith("MOVE:")) {
      String direction = command.substring(5);
      movePlayer(direction);
    }
    else if (command.startsWith("PING")) {
      Serial.println("PONG");
    }
    else if (command.startsWith("POS:")) {
      // Direct position setting (format: POS:x,y)
      int commaPos = command.indexOf(',');
      if (commaPos > 0) {
        int x = command.substring(4, commaPos).toInt();
        int y = command.substring(commaPos + 1).toInt();
        setPlayerPosition(x, y);
      }
    }
  }
}

// Move player based on direction
void movePlayer(String direction) {
  // Store current position
  int oldX = playerX;
  int oldY = playerY;
  
  // Update position based on direction
  if (direction == "UP") {
    playerY--;
  }
  else if (direction == "DOWN") {
    playerY++;
  }
  else if (direction == "LEFT") {
    playerX--;
  }
  else if (direction == "RIGHT") {
    playerX++;
  }
  
  // Check if move is valid (not into a wall or outside the maze)
  if (playerX < 0 || playerX > 7 || playerY < 0 || playerY > 7 || maze[playerY][playerX] == 0) {
    // Invalid move - reset position
    playerX = oldX;
    playerY = oldY;
    Serial.println("MOVE:INVALID");
  }
  else {
    // Valid move
    Serial.print("POS:");
    Serial.print(playerX);
    Serial.print(",");
    Serial.println(playerY);
    
    // Check if player reached exit
    if (playerX == exitX && playerY == exitY) {
      // Victory!
      Serial.println("WIN:TRUE");
      victory();
      
      // Reset player position
      playerX = 1;
      playerY = 1;
    }
  }
}

// Set player position directly
void setPlayerPosition(int x, int y) {
  // Validate position
  if (x >= 0 && x < 8 && y >= 0 && y < 8 && maze[y][x] == 1) {
    playerX = x;
    playerY = y;
    Serial.print("POS:");
    Serial.print(playerX);
    Serial.print(",");
    Serial.println(playerY);
  }
  else {
    Serial.println("POS:INVALID");
  }
}

// Victory animation
void victory() {
  // Flash all LEDs on/off 5 times
  for (int i = 0; i < 5; i++) {
    // All LEDs on
    for (int r = 0; r < 8; r++) {
      digitalWrite(ROW_PINS[r], LOW);
    }
    for (int c = 0; c < 8; c++) {
      digitalWrite(COL_PINS[c], LOW);
    }
    delay(200);
    
    // All LEDs off
    for (int r = 0; r < 8; r++) {
      digitalWrite(ROW_PINS[r], HIGH);
    }
    for (int c = 0; c < 8; c++) {
      digitalWrite(COL_PINS[c], HIGH);
    }
    delay(200);
  }
  
  Serial.println("PLAYER_RESET");
}
