// ENGR40M LED Matrix Maze Game
// Controls an 8x8 LED matrix to display a maze game

// Pin configurations - update these based on your wiring
const byte ROW_PINS[8] = {6, 7, 8, 9, 10, 11, 12, 13};  // Row control pins
const byte COL_PINS[8] = {A3, A2, A1, A0, 5, 4, 3, 2};  // Column control pins

// Game state variables
int playerX = 1;         // Player's X position
int playerY = 1;         // Player's Y position
int exitX = 6;           // Exit X position
int exitY = 6;           // Exit Y position
boolean playerVisible = true;     // Controls player blinking
unsigned long lastBlink = 0;      // Timer for player blink
boolean gameWon = false;          // Game state flag

// Maze layout: 0=path, 1=wall, 2=exit
byte maze[8][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

// Serial communication
String inputBuffer = "";
boolean receivingMaze = false;
int mazeRow = 0;

void setup() {
  Serial.begin(9600);
  
  // Initialize LED matrix pins
  for (int i = 0; i < 8; i++) {
    pinMode(ROW_PINS[i], OUTPUT);
    pinMode(COL_PINS[i], OUTPUT);
    digitalWrite(ROW_PINS[i], HIGH);  // Turn off rows
    digitalWrite(COL_PINS[i], HIGH);  // Turn off columns
  }
  
  // Wait for serial to initialize
  delay(1000);
  
  Serial.println("READY:MAZE_CONTROLLER");
  Serial.println("Ready to receive maze data from Python");
}

void loop() {
  // Toggle player visibility for blinking effect
  if (millis() - lastBlink > 300) {
    playerVisible = !playerVisible;
    lastBlink = millis();
  }
  
  // Check for serial input
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    Serial.print("Command received: ");
    Serial.println(command);
    
    // Check for different command types
    if (command.startsWith("MAZE:")) {
      // Received a maze update
      parseMazeData(command.substring(5));
      // Reset game state when new maze is received
      gameWon = false;
    }
    else if (command.startsWith("PLAYER:")) {
      // Received a player position update
      parsePlayerPosition(command.substring(7));
    }
    else if (command.startsWith("WIN:")) {
      // Player won the game
      Serial.println("Victory triggered!");
      gameWon = true;
      victory();
      // Send acknowledgement
      Serial.println("WIN:ACKNOWLEDGED");
    }
    else if (command.startsWith("RESET:")) {
      // Reset command received
      gameWon = false;
      Serial.println("Game reset");
    }
    // Check if it's a ping
    else if (command.startsWith("PING")) {
      Serial.println("PONG");
    }
  }
  
  // If game won, show victory animation briefly then display maze
  if (gameWon) {
    // Show victory animation for 3 seconds after winning
    if (millis() - lastBlink < 3000) {
      if (millis() % 300 < 150) { // Fast blinking during victory
        victory();
      }
    } else {
      // After 3 seconds, go back to showing the maze
      displayMaze();
    }
  } else {
    // Normal gameplay - display maze
    displayMaze();
  }
}

// Draw a border around the display edges
void displayBorder() {
  for (int row = 0; row < 8; row++) {
    // Turn off all rows and columns
    for (int r = 0; r < 8; r++) digitalWrite(ROW_PINS[r], HIGH);
    for (int c = 0; c < 8; c++) digitalWrite(COL_PINS[c], HIGH);
    
    // Enable current row
    digitalWrite(ROW_PINS[row], LOW);
    
    // Light up border LEDs (top/bottom rows or first/last columns)
    if (row == 0 || row == 7) {
      for (int col = 0; col < 8; col++) digitalWrite(COL_PINS[col], LOW);
    } else {
      digitalWrite(COL_PINS[0], LOW);
      digitalWrite(COL_PINS[7], LOW);
    }
    
    delay(1);
  }
}

// Blink a single LED at position (3,3)
void displaySingleLED() {
  static unsigned long lastBlink = 0;
  static boolean ledOn = true;
  
  // Toggle LED state every 500ms
  if (millis() - lastBlink > 500) {
    ledOn = !ledOn;
    lastBlink = millis();
  }
  
  if (ledOn) {
    // Clear display
    for (int r = 0; r < 8; r++) digitalWrite(ROW_PINS[r], HIGH);
    for (int c = 0; c < 8; c++) digitalWrite(COL_PINS[c], HIGH);
    
    // Light up LED at (3,3)
    digitalWrite(ROW_PINS[3], LOW);
    digitalWrite(COL_PINS[3], LOW);
    delay(1);
  }
}

// Cycle through lighting up each row
void displayRows() {
  static int currentRow = 0;
  static unsigned long lastChange = 0;
  
  // Move to next row every 200ms
  if (millis() - lastChange > 200) {
    currentRow = (currentRow + 1) % 8;
    lastChange = millis();
  }
  
  // Clear display
  for (int r = 0; r < 8; r++) digitalWrite(ROW_PINS[r], HIGH);
  
  // Light up current row
  for (int c = 0; c < 8; c++) digitalWrite(COL_PINS[c], LOW);
  digitalWrite(ROW_PINS[currentRow], LOW);
  
  delay(1);
}

// Cycle through lighting up each column
void displayColumns() {
  static int currentCol = 0;
  static unsigned long lastChange = 0;
  
  // Move to next column every 200ms
  if (millis() - lastChange > 200) {
    currentCol = (currentCol + 1) % 8;
    lastChange = millis();
  }
  
  // Clear display
  for (int c = 0; c < 8; c++) digitalWrite(COL_PINS[c], HIGH);
  
  // Light up current column
  for (int r = 0; r < 8; r++) digitalWrite(ROW_PINS[r], LOW);
  digitalWrite(COL_PINS[currentCol], LOW);
  
  delay(1);
}

// Display an animated checkerboard pattern
void displayCheckerboard() {
  static unsigned long lastToggle = 0;
  static boolean toggle = false;
  
  // Toggle pattern every 500ms
  if (millis() - lastToggle > 500) {
    toggle = !toggle;
    lastToggle = millis();
  }
  
  // Draw checkerboard pattern
  for (int row = 0; row < 8; row++) {
    // Clear rows
    for (int r = 0; r < 8; r++) digitalWrite(ROW_PINS[r], HIGH);
    
    // Set column pattern (alternating based on row + toggle)
    for (int col = 0; col < 8; col++) {
      digitalWrite(COL_PINS[col], (row + col + toggle) % 2 ? HIGH : LOW);
    }
    
    // Enable current row
    digitalWrite(ROW_PINS[row], LOW);
    delay(1);
  }
}

// Display the maze game
void displayMaze() {
  static unsigned long lastDisplay = 0;
  static int currentRow = 0;
  
  // Only update display every 2ms to prevent flicker (500Hz refresh rate)
  if (millis() - lastDisplay < 2) {
    return;
  }
  lastDisplay = millis();
  
  // Turn off all rows and columns
  for (int r = 0; r < 8; r++) {
    digitalWrite(ROW_PINS[r], HIGH);  // Rows off
  }
  for (int c = 0; c < 8; c++) {
    digitalWrite(COL_PINS[c], HIGH);  // Columns off
  }
  
  // Handle player blinking (every 500ms)
  if (millis() - lastBlink > 500) {
    playerVisible = !playerVisible;
    lastBlink = millis();
  }
  
  // Handle exit blinking (same timing as player)
  bool exitVisible = (millis() / 500) % 2 == 0;  // Blink every 500ms
  
  // Display walls and exit for current row
  bool shouldShowExit = (millis() / 500) % 2 == 0;  // Blink every 500ms
  
  // First, turn off all columns
  for (int c = 0; c < 8; c++) {
    digitalWrite(COL_PINS[c], HIGH);  // Columns off
  }
  
  // For 90-degree rotation: swap rows and columns and adjust orientation
  // Original (x,y) becomes (7-y, x) for 90-degree clockwise rotation
  for (int col = 0; col < 8; col++) {
    // Calculate rotated position
    int rotatedRow = 7 - col;  // For 90-degree rotation
    int rotatedCol = currentRow;
    
    // Get the value at the rotated position
    byte cellValue = maze[col][currentRow];
    
    if (cellValue == 1) {  // Wall
      digitalWrite(COL_PINS[rotatedRow], LOW);
    } else if (cellValue == 2 && shouldShowExit) {  // Exit (blinking)
      digitalWrite(COL_PINS[rotatedRow], LOW);
    }
  }
  
  // Enable current row (inverted for correct orientation)
  digitalWrite(ROW_PINS[7 - currentRow], LOW);
  
  // Handle player position with rotation
  // Original (playerX, playerY) becomes (7-playerY, playerX)
  int rotatedPlayerX = 7 - playerY;
  int rotatedPlayerY = playerX;
  
  // If player is in current row and should be visible, show player
  if (rotatedPlayerY == currentRow && playerVisible) {
    digitalWrite(COL_PINS[rotatedPlayerX], LOW);
  }
  
  // Move to next row for next refresh
  currentRow = (currentRow + 1) % 8;
  
  // Small delay for LED to be visible
  delayMicroseconds(100);
  
  // Turn off the current row after display
  digitalWrite(ROW_PINS[(currentRow + 7) % 8], HIGH);
}

// Parse maze data from JSON string
void parseMazeData(String jsonData) {
  // Debug output
  Serial.print("Received maze data: ");
  Serial.println(jsonData);
  
  // Reset the maze to all walls first
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      maze[y][x] = 1; // Default to wall
    }
  }

  // Keep track of array position
  int row = 0;
  int col = 0;

  // Process each character in the JSON data
  for (int i = 0; i < jsonData.length(); i++) {
    char c = jsonData.charAt(i);
    
    // Look for digits (0,1,2) in the string
    if (c >= '0' && c <= '2') {
      if (col < 8 && row < 8) {
        // Invert coordinates to match LED matrix orientation
        int displayRow = 7 - row;
        int displayCol = 7 - col;
        
        maze[displayRow][displayCol] = c - '0'; // Convert char to int
        
        // Update exit position if found
        if (maze[displayRow][displayCol] == 2) {
          exitX = displayCol;
          exitY = displayRow;
        }
        
        // Move to next column
        col++;
        
        // If we've filled a row, move to the next
        if (col >= 8) {
          col = 0;
          row++;
        }
      }
    }
  }
  
  Serial.println("MAZE:RECEIVED");
}

// Update player position from "x,y" string
void parsePlayerPosition(String posData) {
  int comma = posData.indexOf(',');
  if (comma > 0) {
    // Parse coordinates
    int newX = 7 - posData.substring(0, comma).toInt();  // Invert X
    int newY = 7 - posData.substring(comma + 1).toInt(); // Invert Y
    
    // Validate and update position
    if (newX >= 0 && newX < 8 && newY >= 0 && newY < 8) {
      playerX = newX;
      playerY = newY;
      playerVisible = true;
      lastBlink = millis();
      Serial.println("PLAYER:UPDATED");
    } else {
      Serial.println("ERR:INVALID_POSITION");
    }
  } else {
    Serial.println("ERR:INVALID_FORMAT");
  }
}

// Flash all LEDs for victory animation
void victory() {
  static int flashCount = 0;
  static unsigned long lastFlash = 0;
  
  // Flash 3 times (6 states: on/off/on/off/on/off)
  if (flashCount >= 6) {
    flashCount = 0;
    return;
  }
  
  // Toggle all LEDs every 150ms
  if (millis() - lastFlash > 150) {
    lastFlash = millis();
    bool ledsOn = (flashCount % 2 == 0);
    
    // Set all rows and columns at once
    for (int i = 0; i < 8; i++) {
      digitalWrite(ROW_PINS[i], ledsOn ? LOW : HIGH);
      digitalWrite(COL_PINS[i], ledsOn ? LOW : HIGH);
    }
    
    flashCount++;
  }
}
