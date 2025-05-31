const byte ROW_PINS[8] = {6, 7, 8, 9, 10, 11, 12, 13};  // Row pins (LOW = row on)
const byte COL_PINS[8] = {A3, A2, A1, A0, 5, 4, 3, 2};  // Column pins (LOW = LED on)


// Player position and maze data
int playerX = 1;
int playerY = 1;
int exitX = 6;
int exitY = 6;
boolean playerVisible = true;
unsigned long lastBlink = 0;
boolean gameWon = false;

// Simple maze layout (0 = path, 1 = wall, 2 = exit)
byte maze[8][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},  
  {0, 0, 0, 0, 0, 0, 0, 0},  // Paths are 0 (ON)
  {0, 0, 0, 0, 0, 0, 0, 0},  // Walls are 1 (OFF)
  {0, 0, 0, 0, 0, 0, 0, 0},  // Exit is 2 (ON, blinking)
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

// Buffer for receiving serial data
String inputBuffer = "";
boolean receivingMaze = false;
int mazeRow = 0;

void setup() {
  Serial.begin(9600);
  
  // Reset all pins
  for (int i = 0; i < 8; i++) {
    pinMode(ROW_PINS[i], OUTPUT);
    pinMode(COL_PINS[i], OUTPUT);
    digitalWrite(ROW_PINS[i], HIGH);  // Rows off 
    digitalWrite(COL_PINS[i], HIGH);  // Columns off
  }
  
  // Wait for serial to initialize
  delay(1000);
  
  Serial.println("READY:MAZE_CONTROLLER");
  Serial.println("Ready to receive maze data from Python");
}

void loop() {
  // Handle player blinking
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

// Display just the border
void displayBorder() {
  // Turn off all LEDs first
  for (int r = 0; r < 8; r++) {
    digitalWrite(ROW_PINS[r], HIGH);  // All rows off
  }
  
  // Activate each row one at a time, light up border LEDs
  for (int row = 0; row < 8; row++) {
    // All columns off first
    for (int col = 0; col < 8; col++) {
      digitalWrite(COL_PINS[col], HIGH);
    }
    
    // Enable this row
    digitalWrite(ROW_PINS[row], LOW);
    
    // Light up border LEDs on this row
    if (row == 0 || row == 7) {
      // Top or bottom row - all LEDs on
      for (int col = 0; col < 8; col++) {
        digitalWrite(COL_PINS[col], LOW);
      }
    } else {
      // Middle row - just edge LEDs
      digitalWrite(COL_PINS[0], LOW);  // Left edge
      digitalWrite(COL_PINS[7], LOW);  // Right edge
    }
    
    // Hold for visibility
    delay(1);
    
    // Disable this row before moving to next
    digitalWrite(ROW_PINS[row], HIGH);
  }
}

// Display a single blinking LED at position 3,3
void displaySingleLED() {
  static unsigned long lastBlink = 0;
  static boolean ledOn = true;
  
  // Blink every 500ms
  if (millis() - lastBlink > 500) {
    ledOn = !ledOn;
    lastBlink = millis();
  }
  
  // Display the LED
  for (int i = 0; i < 10; i++) {  // Refresh several times for brightness
    // Turn off all rows and columns
    for (int r = 0; r < 8; r++) {
      digitalWrite(ROW_PINS[r], HIGH);
    }
    for (int c = 0; c < 8; c++) {
      digitalWrite(COL_PINS[c], HIGH);
    }
    
    if (ledOn) {
      // Activate row 3
      digitalWrite(ROW_PINS[3], LOW);
      
      // Activate column 3
      digitalWrite(COL_PINS[3], LOW);
      
      // Hold briefly
      delay(1);
    }
  }
}

// Light up one row at a time
void displayRows() {
  static int currentRow = 0;
  static unsigned long lastChange = 0;
  
  // Change row every 200ms
  if (millis() - lastChange > 200) {
    currentRow = (currentRow + 1) % 8;
    lastChange = millis();
  }
  
  // Display the current row
  for (int i = 0; i < 10; i++) {  // Refresh several times
    // Turn off all rows
    for (int r = 0; r < 8; r++) {
      digitalWrite(ROW_PINS[r], HIGH);
    }
    
    // Turn on all columns
    for (int c = 0; c < 8; c++) {
      digitalWrite(COL_PINS[c], LOW);
    }
    
    // Activate current row
    digitalWrite(ROW_PINS[currentRow], LOW);
    
    // Hold briefly
    delay(1);
  }
}

// Light up one column at a time
void displayColumns() {
  static int currentCol = 0;
  static unsigned long lastChange = 0;
  
  // Change column every 200ms
  if (millis() - lastChange > 200) {
    currentCol = (currentCol + 1) % 8;
    lastChange = millis();
  }
  
  // Display the current column
  for (int i = 0; i < 10; i++) {  // Refresh several times
    // Turn on all rows
    for (int r = 0; r < 8; r++) {
      digitalWrite(ROW_PINS[r], LOW);
    }
    
    // Turn off all columns
    for (int c = 0; c < 8; c++) {
      digitalWrite(COL_PINS[c], HIGH);
    }
    
    // Activate current column
    digitalWrite(COL_PINS[currentCol], LOW);
    
    // Hold briefly
    delay(1);
    
    // Turn off all rows before next iteration
    for (int r = 0; r < 8; r++) {
      digitalWrite(ROW_PINS[r], HIGH);
    }
  }
}

// Display a checkerboard pattern
void displayCheckerboard() {
  static unsigned long lastToggle = 0;
  static boolean toggle = false;
  
  // Toggle pattern every 500ms
  if (millis() - lastToggle > 500) {
    toggle = !toggle;
    lastToggle = millis();
  }
  
  // Scan through all rows
  for (int row = 0; row < 8; row++) {
    // All rows off
    for (int r = 0; r < 8; r++) {
      digitalWrite(ROW_PINS[r], HIGH);
    }
    
    // Set column pattern
    for (int col = 0; col < 8; col++) {
      if ((row + col + toggle) % 2 == 0) {
        digitalWrite(COL_PINS[col], LOW);   // LED on
      } else {
        digitalWrite(COL_PINS[col], HIGH);  // LED off
      }
    }
    
    // Enable this row
    digitalWrite(ROW_PINS[row], LOW);
    
    // Hold briefly
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
    
    // Skip all characters except digits and use 0 and 1 to set maze
    if (c == '0' || c == '1' || c == '2') {
      // If it's a digit (0 or 1), use it to set the maze
      if (col < 8 && row < 8) {
        // INVERT X AND Y COORDINATES TO MATCH PLAYER POSITION INVERSION
        int displayRow = 7 - row;
        int displayCol = 7 - col;
        
        if (c == '0') {
          maze[displayRow][displayCol] = 0; // Path
        } else if (c == '1') {
          maze[displayRow][displayCol] = 1; // Wall
        } else if (c == '2') {
          maze[displayRow][displayCol] = 0; // Path (exit position)
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
  
  // Print the parsed maze for debugging
  Serial.println("Parsed maze:");
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      Serial.print(maze[y][x]);
      Serial.print(" ");
    }
    Serial.println();
  }
  
  // Find the exit position (value 2)
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (maze[y][x] == 2) {
        exitX = x;
        exitY = y;
        break;
      }
    }
  }
  
  Serial.println("MAZE:RECEIVED");
}

// Parse player position from format "x,y"
void parsePlayerPosition(String posData) {
  // Debug output
  Serial.print("Received player position: ");
  Serial.println(posData);
  
  // Look for comma separator
  int commaIndex = posData.indexOf(',');
  if (commaIndex > 0) {
    String xStr = posData.substring(0, commaIndex);
    String yStr = posData.substring(commaIndex + 1);
    
    // Convert to integers and store
    int newX = xStr.toInt();
    int newY = yStr.toInt();
    
    // Check if valid
    if (newX >= 0 && newX < 8 && newY >= 0 && newY < 8) {
      // FIX INVERTED DIRECTIONS: Invert the X and Y coordinates
      playerX = 7 - newX;  // Invert X coordinate
      playerY = 7 - newY;  // Invert Y coordinate
      
      // Reset the blink timer to make player visible immediately
      playerVisible = true;
      lastBlink = millis();
      
      // Debug output
      Serial.print("Player moved to (inverted): ");
      Serial.print(playerX);
      Serial.print(",");
      Serial.println(playerY);
    } else {
      Serial.println("Invalid player position!");
    }
    
    // Acknowledge update
    Serial.println("PLAYER:UPDATED");
  } else {
    Serial.println("Invalid player position format!");
  }
}

// Victory animation
void victory() {
  static int flashCount = 0;
  static unsigned long lastFlash = 0;
  
  // Only flash 3 times, then return to normal display
  if (flashCount >= 6) { // 3 on/off cycles
    flashCount = 0;
    return;
  }
  
  // Alternate between all LEDs on and all LEDs off
  if (millis() - lastFlash > 150) { // Faster flashing
    lastFlash = millis();
    flashCount++;
    
    if (flashCount % 2 == 1) {
      // All LEDs on
      for (int r = 0; r < 8; r++) {
        digitalWrite(ROW_PINS[r], LOW);
      }
      for (int c = 0; c < 8; c++) {
        digitalWrite(COL_PINS[c], LOW);
      }
    } else {
      // All LEDs off
      for (int r = 0; r < 8; r++) {
        digitalWrite(ROW_PINS[r], HIGH);
      }
      for (int c = 0; c < 8; c++) {
        digitalWrite(COL_PINS[c], HIGH);
      }
    }
  }
}
