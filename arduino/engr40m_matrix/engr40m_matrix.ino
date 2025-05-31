/*
 * ENGR40M LED Matrix Driver
 * Specifically configured for the lab PCB with pMOS row drivers
 */

// Pin definitions - match these to your actual connections
const byte ROW_PINS[8] = {6, 7, 8, 9, 10, 11, 12, 13};  // Connected to pMOS transistors
const byte COL_PINS[8] = {A3, A2, A1, A0, 5, 4, 3, 2};  // Direct to LED cathodes

// Maze representation (1 = wall, 0 = path)
// IMPORTANT: For this matrix, walls=0 (off) and paths=1 (on)
byte maze[8][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},  // Top border all walls (off)
  {0, 1, 1, 1, 1, 1, 1, 0},  // Left/right walls off, path on
  {0, 1, 0, 0, 1, 0, 1, 0},
  {0, 1, 1, 1, 1, 1, 1, 0},
  {0, 1, 0, 0, 1, 0, 1, 0},
  {0, 1, 1, 1, 1, 0, 1, 0},
  {0, 1, 1, 1, 1, 1, 1, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}   // Bottom border all walls (off)
};

// Player position
int playerX = 1;
int playerY = 1;
int exitX = 6;
int exitY = 6;
boolean playerVisible = true;
unsigned long lastBlink = 0;

void setup() {
  Serial.begin(9600);
  
  // Configure all pins
  // ROWS: OUTPUT, start HIGH (pMOS off)
  // COLS: OUTPUT, start LOW (cathodes off)
  for (int i = 0; i < 8; i++) {
    pinMode(ROW_PINS[i], OUTPUT);
    digitalWrite(ROW_PINS[i], HIGH);  // pMOS off (row inactive)
    
    pinMode(COL_PINS[i], OUTPUT);
    digitalWrite(COL_PINS[i], LOW);   // Cathodes off
  }
  
  Serial.println("ENGR40M Matrix Driver Ready");
}

void loop() {
  // Handle player blinking
  if (millis() - lastBlink > 300) {
    lastBlink = millis();
    playerVisible = !playerVisible;
  }
  
  // Handle serial commands
  checkSerialCommands();
  
  // Display the maze one row at a time (proper scanning)
  displayMaze();
}

// Display the maze using proper row-by-row scanning
void displayMaze() {
  // Force player to blink more visibly by toggling every 300ms
  static unsigned long lastToggle = 0;
  if (millis() - lastToggle > 300) {
    playerVisible = !playerVisible;
    lastToggle = millis();
  }
  
  // Scan through each row one at a time
  for (int row = 0; row < 8; row++) {
    // First make sure all rows are off (HIGH for pMOS)
    for (int i = 0; i < 8; i++) {
      digitalWrite(ROW_PINS[i], HIGH);  // All rows off
    }
    
    // Set up the column pins for this row - with our maze 0=wall(off), 1=path(on)
    for (int col = 0; col < 8; col++) {
      if (row == playerY && col == playerX) {
        // Player position (blinking)
        if (playerVisible) {
          digitalWrite(COL_PINS[col], LOW);   // Player visible (LED ON)
        } else {
          digitalWrite(COL_PINS[col], HIGH);  // Player hidden (LED OFF)
        }
      }
      else if (row == exitY && col == exitX) {
        // Exit position - always on
        digitalWrite(COL_PINS[col], LOW);    // LED ON
      }
      else if (maze[row][col] == 1) {
        // Path = 1 = LED ON
        digitalWrite(COL_PINS[col], LOW);
      }
      else {
        // Wall = 0 = LED OFF
        digitalWrite(COL_PINS[col], HIGH);
      }
    }
    
    // Now enable this row by setting its pin LOW (pMOS on)
    digitalWrite(ROW_PINS[row], LOW);
    
    // Hold this row pattern visible longer for better visibility
    delayMicroseconds(1000);  // Even longer delay for brightness
    
    // Turn off this row before moving to next row
    digitalWrite(ROW_PINS[row], HIGH);
    
    // Reset all columns to prevent ghosting
    for (int col = 0; col < 8; col++) {
      digitalWrite(COL_PINS[col], HIGH); // All LEDs off
    }
  }
}

// Handle incoming serial commands
void checkSerialCommands() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd.startsWith("MOVE:")) {
      String direction = cmd.substring(5);
      
      // Store old position
      int oldX = playerX;
      int oldY = playerY;
      
      // Update position based on direction
      if (direction == "UP") playerY--;
      else if (direction == "DOWN") playerY++;
      else if (direction == "LEFT") playerX--;
      else if (direction == "RIGHT") playerX++;
      
      // Check boundaries and walls
      if (playerX < 0 || playerX > 7 || playerY < 0 || playerY > 7 || maze[playerY][playerX] == 1) {
        // Invalid move - reset position
        playerX = oldX;
        playerY = oldY;
      }
      
      // Report position
      Serial.print("Player at: ");
      Serial.print(playerX);
      Serial.print(",");
      Serial.println(playerY);
      
      // Check for win
      if (playerX == exitX && playerY == exitY) {
        Serial.println("YOU WIN!");
        
        // Victory animation
        for (int i = 0; i < 5; i++) {
          // Flash all LEDs on
          for (int r = 0; r < 8; r++) {
            digitalWrite(ROW_PINS[r], LOW);  // All rows on
          }
          for (int c = 0; c < 8; c++) {
            digitalWrite(COL_PINS[c], LOW);  // All columns on (LEDs on)
          }
          delay(200);  // Stay on for 200ms
          
          // All LEDs off
          for (int r = 0; r < 8; r++) {
            digitalWrite(ROW_PINS[r], HIGH); // All rows off
          }
          delay(200);  // Stay off for 200ms
        }
        
        // Reset player position
        playerX = 1;
        playerY = 1;
      }
    }
    else if (cmd.startsWith("TEST")) {
      // Run test pattern
      runTestPattern();
    }
  }
}

// Simple test pattern to verify display
void runTestPattern() {
  Serial.println("Running test pattern...");
  
  // 1. Border test
  for (int i = 0; i < 3; i++) {
    // Clear the display
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        maze[row][col] = 0;
      }
    }
    
    // Set the border
    for (int i = 0; i < 8; i++) {
      maze[0][i] = 1;  // Top row
      maze[7][i] = 1;  // Bottom row
      maze[i][0] = 1;  // Left column
      maze[i][7] = 1;  // Right column
    }
    
    // Display for a moment
    for (int j = 0; j < 50; j++) {
      displayMaze();
    }
    
    // Clear display
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        maze[row][col] = 0;
      }
    }
    
    // Display for a moment
    for (int j = 0; j < 20; j++) {
      displayMaze();
    }
  }
  
  // Restore the original maze
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      if (row == 0 || row == 7 || col == 0 || col == 7) {
        maze[row][col] = 1;
      } else {
        maze[row][col] = 0;
      }
    }
  }
  
  // Add some internal walls
  maze[2][2] = 1; maze[2][3] = 1; maze[2][5] = 1;
  maze[4][2] = 1; maze[4][3] = 1; maze[4][5] = 1;
  maze[5][5] = 1;
  
  Serial.println("Test complete");
}
