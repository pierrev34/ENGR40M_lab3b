/*
 * Simple LED Matrix Maze Game
 * For ENGR40M Lab 3B
 */

// LED Matrix pins
const byte ROW_PINS[8] = {6, 7, 8, 9, 10, 11, 12, 13};
const byte COL_PINS[8] = {2, 3, 4, 5, A0, A1, A2, A3};

// Game variables
int playerX = 1;
int playerY = 1;
int exitX = 6;
int exitY = 6;

// Simple maze with walls (1), path (0), player position (2), and exit (3)
int maze[8][8] = {
  {1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 0, 1, 0, 1},
  {1, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 1, 0, 0, 0, 1},
  {1, 0, 0, 1, 1, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1}
};

// Timing variables
unsigned long lastMoveTime = 0;
unsigned long lastBlinkTime = 0;
boolean playerVisible = true;

// Current direction of movement (used for continuous movement)
int currentDirection = 0; // 0=none, 1=up, 2=right, 3=down, 4=left

void setup() {
  // Initialize all LED matrix pins
  for (byte i = 0; i < 8; i++) {
    pinMode(ROW_PINS[i], OUTPUT);
    pinMode(COL_PINS[i], OUTPUT);
    digitalWrite(ROW_PINS[i], HIGH);   // Turn off rows initially
    digitalWrite(COL_PINS[i], HIGH);  // Turn off columns initially
  }
  
  // Initialize serial communication for debugging
  Serial.begin(9600);
  
  // Initialize player and exit positions in maze
  maze[playerY][playerX] = 2;  // Player
  maze[exitY][exitX] = 3;      // Exit
  
  // Send startup message
  Serial.println("MAZE GAME READY");
}

void loop() {
  // Check for serial commands
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Process commands
    if (command.startsWith("MOVE:")) {
      String direction = command.substring(5);
      movePlayer(direction);
    }
    else if (command.startsWith("POS:")) {
      // Direct position control format: POS:x,y
      int commaPos = command.indexOf(',');
      if (commaPos > 0) {
        int x = command.substring(4, commaPos).toInt();
        int y = command.substring(commaPos + 1).toInt();
        setPlayerPosition(x, y);
      }
    }
  }
  
  // Check button inputs (active LOW with internal pull-ups)
  checkButtonInputs();
  
  // Handle player blinking
  if (millis() - lastBlinkTime > 300) {
    lastBlinkTime = millis();
    playerVisible = !playerVisible;
  }
  
  // Check for button presses (every 100ms to prevent too rapid movement)
  if (millis() - lastButtonCheck > 100) {
    lastButtonCheck = millis();
    checkButtons();
  }
  
  // Display the LED matrix
  refreshMatrix();
  
  // Check for incoming serial data
  readSerialData();
  
  // Send status and request web commands every second
  if (millis() - lastSent > 1000) {
    lastSent = millis();
    Serial.print("READY:");
    Serial.println(counter++);
    
    // Send player position (for web sync)
    Serial.print("POS:");
    Serial.print(playerX);
    Serial.print(",");
    Serial.println(playerY);
  }
}

// Check button states and move player if needed
void checkButtons() {
  // Read current button states
  btnUpState = digitalRead(BTN_UP);
  btnDownState = digitalRead(BTN_DOWN);
  btnLeftState = digitalRead(BTN_LEFT);
  btnRightState = digitalRead(BTN_RIGHT);
  
  // Print button states for debugging
  static unsigned long lastDebugPrint = 0;
  if (millis() - lastDebugPrint > 1000) {
    lastDebugPrint = millis();
    Serial.print("Button states: ");
    Serial.print(btnUpState);
    Serial.print(btnDownState);
    Serial.print(btnLeftState);
    Serial.println(btnRightState);
  }
  
  // Directly check for button press (LOW = pressed with pull-up)
  if (btnUpState == LOW) {
    movePlayer(0, -1); // Move up
    Serial.println("Button UP active");
    delay(100); // Debounce
  }
  
  // Check for DOWN button press
  if (btnDownState == LOW) {
    movePlayer(0, 1); // Move down
    Serial.println("Button DOWN active");
    delay(100); // Debounce
  }
  
  // Check for LEFT button press
  if (btnLeftState == LOW) {
    movePlayer(-1, 0); // Move left
    Serial.println("Button LEFT active");
    delay(100); // Debounce
  }
  
  // Check for RIGHT button press
  if (btnRightState == LOW) {
    movePlayer(1, 0); // Move right
    Serial.println("Button RIGHT active");
    delay(100); // Debounce
  }
  
  // Update previous button states
  btnUpPrev = btnUpState;
  btnDownPrev = btnDownState;
  btnLeftPrev = btnLeftState;
  btnRightPrev = btnRightState;
}

// Move player by delta_x, delta_y
void movePlayer(int delta_x, int delta_y) {
  // Calculate new position
  int newX = playerX + delta_x;
  int newY = playerY + delta_y;
  
  // Bounds checking
  if (newX < 0 || newX > 7 || newY < 0 || newY > 7) {
    return; // Don't move if out of bounds
  }
  
  // Don't move into walls
  if (mazeLeds[newY][newX] == 1) {
    return;
  }
  
  // Update player position
  playerX = newX;
  playerY = newY;
  
  // Debug output
  Serial.print("Player moved to ");
  Serial.print(playerX);
  Serial.print(",");
  Serial.println(playerY);
  
  // Update player in the maze display
  updatePlayerPosition();
  
  // Check if player reached exit
  if (playerX == exitX && playerY == exitY) {
    winAnimation();
    playerX = 1;
    playerY = 1;
    updatePlayerPosition();
  }
}

// Set up a test pattern in the matrix
void setupTestPattern() {
  // Clear all LEDs
  clearMatrix();
  
  // Set a border
  for (int i = 0; i < 8; i++) {
    mazeLeds[0][i] = 1;  // Top border
    mazeLeds[7][i] = 1;  // Bottom border
    mazeLeds[i][0] = 1;  // Left border
    mazeLeds[i][7] = 1;  // Right border
  }
  
  // Set player and exit positions
  mazeLeds[playerY][playerX] = 2;  // Player at (1,1)
  mazeLeds[exitY][exitX] = 3;      // Exit at (6,6)
}

// Clear the LED matrix
void clearMatrix() {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      mazeLeds[y][x] = 0;
    }
  }
}

// Simple LED matrix refreshing (no multiplexing)
void refreshMatrix() {
  // Drive one LED at a time for maximum brightness
  for (byte row = 0; row < 8; row++) {
    for (byte col = 0; col < 8; col++) {
      // Skip if LED should be off
      if (mazeLeds[row][col] == 0) continue;
      
      // Turn off all pins first
      for (byte i = 0; i < 8; i++) {
        digitalWrite(PLUS_PINS[i], HIGH);
        digitalWrite(MINUS_PINS[i], HIGH);
      }
      
      // Special handling for exit position - slower blinking
      if (mazeLeds[row][col] == 3) { // Exit position
        // Turn on this single LED
        digitalWrite(PLUS_PINS[row], LOW);
        digitalWrite(MINUS_PINS[col], LOW);
        delayMicroseconds(500);
      }
      // Special handling for player position - blinking
      else if (mazeLeds[row][col] == 2) { // Player position
        if (playerVisible) {
          // Turn on this single LED
          digitalWrite(PLUS_PINS[row], LOW);
          digitalWrite(MINUS_PINS[col], LOW);
          delayMicroseconds(500);
        }
      } 
      // Normal walls
      else {
        // Turn on this single LED
        digitalWrite(PLUS_PINS[row], LOW);
        digitalWrite(MINUS_PINS[col], LOW);
        delayMicroseconds(100);
      }
    }
  }
}

// Process incoming serial data
void readSerialData() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Echo back the command
    Serial.print("RECEIVED:");
    Serial.println(command);
    
    // Handle move commands
    if (command.startsWith("MOVE:")) {
      String direction = command.substring(5);
      
      // Force player movement
      if (direction == "UP") {
        movePlayer(0, -1);
        Serial.println("Manual UP move");
      }
      else if (direction == "DOWN") {
        movePlayer(0, 1);
        Serial.println("Manual DOWN move");
      }
      else if (direction == "LEFT") {
        movePlayer(-1, 0);
        Serial.println("Manual LEFT move");
      }
      else if (direction == "RIGHT") {
        movePlayer(1, 0);
        Serial.println("Manual RIGHT move");
      }
    }
    
    // Force direct positioning for debugging
    else if (command.startsWith("POS:")) {
      String posData = command.substring(4);
      int commaPos = posData.indexOf(',');
      if (commaPos > 0) {
        int x = posData.substring(0, commaPos).toInt();
        int y = posData.substring(commaPos + 1).toInt();
        
        // Force position update
        playerX = constrain(x, 0, 7);
        playerY = constrain(y, 0, 7);
        updatePlayerPosition();
        Serial.print("Forced position to ");
        Serial.print(playerX);
        Serial.print(",");
        Serial.println(playerY);
      }
    }
    
    // Handle keyboard input (for testing)
    else if (command.startsWith("KEY:")) {
      char key = command.charAt(4);
      int oldX = playerX;
      int oldY = playerY;
      
      // Process key
      if (key == 'w' || key == 'W') playerY--;
      else if (key == 's' || key == 'S') playerY++;
      else if (key == 'a' || key == 'A') playerX--;
      else if (key == 'd' || key == 'D') playerX++;
      
      // Check boundaries
      if (playerX < 0 || playerX > 7 || playerY < 0 || playerY > 7) {
        playerX = oldX;
        playerY = oldY;
      }
      
      // Update player position
      updatePlayerPosition();
    }
    
    // Receive maze data from web
    else if (command.startsWith("MAZE:")) {
      String data = command.substring(5);
      loadMazeFromString(data);
    }
  }
}

// Update player position in the LED matrix
void updatePlayerPosition() {
  // Clear the old player position on the entire board
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (mazeLeds[y][x] == 2) {
        mazeLeds[y][x] = 0; // Clear player
      }
    }
  }
  
  // Set the new player position
  mazeLeds[playerY][playerX] = 2;
}

// Play win animation
void winAnimation() {
  // Flash the entire matrix 3 times
  for (int flash = 0; flash < 3; flash++) {
    // Turn all LEDs on
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        mazeLeds[y][x] = 1;
      }
    }
    
    // Refresh for 200ms
    unsigned long startTime = millis();
    while (millis() - startTime < 200) {
      refreshMatrix();
    }
    
    // Turn all LEDs off
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        mazeLeds[y][x] = 0;
      }
    }
    
    // Refresh for 200ms
    startTime = millis();
    while (millis() - startTime < 200) {
      refreshMatrix();
    }
  }
  
  // Restore the border
  setupTestPattern();
}

// Load maze data from a string
void loadMazeFromString(String data) {
  // Clear existing maze
  clearMatrix();
  
  // Parse the data and update maze
  int index = 0;
  for (int y = 0; y < 8 && index < data.length(); y++) {
    for (int x = 0; x < 8 && index < data.length(); x++) {
      char c = data.charAt(index++);
      if (c == '1') {
        mazeLeds[y][x] = 1; // Wall
      }
    }
  }
  
  // Set player and exit
  mazeLeds[playerY][playerX] = 2;
  mazeLeds[exitY][exitX] = 3;
}
