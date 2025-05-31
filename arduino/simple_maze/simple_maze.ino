/*
 * Simple LED Matrix Maze Game
 * For ENGR40M Lab 3B
 */

// LED Matrix pins - VERIFY THESE MATCH YOUR HARDWARE
// For common anode matrix:
const byte ROW_PINS[8] = {6, 7, 8, 9, 10, 11, 12, 13};  // Connect to row pins (ROW LOW = ON)
const byte COL_PINS[8] = {A3, A2, A1, A0, 5, 4, 3, 2};  // Connect to column pins (COL HIGH = ON)

// Game variables
int playerX = 1;
int playerY = 1;
int exitX = 6;
int exitY = 6;
boolean playerVisible = true;

// Button pins - directly connected to COL_PINS
#define BTN_UP    2  // Same as COL_PINS[7]
#define BTN_RIGHT 3  // Same as COL_PINS[6]
#define BTN_DOWN  4  // Same as COL_PINS[5]
#define BTN_LEFT  5  // Same as COL_PINS[4]

// Timing variables
unsigned long lastBlinkTime = 0;
unsigned long lastMoveTime = 0;

// Hard-coded maze layout: 1=wall, 0=path
// This simple maze has walls all around the border and some internal walls
byte maze[8][8] = {
  {1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 1},
  {1, 0, 1, 0, 1, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1}
};

void setup() {
  Serial.begin(9600);
  
  // Set all row pins as outputs (HIGH = off)
  for (int i = 0; i < 8; i++) {
    pinMode(ROW_PINS[i], OUTPUT);
    digitalWrite(ROW_PINS[i], HIGH);
  }
  
  // Set all column pins as outputs (LOW = off)
  for (int i = 0; i < 8; i++) {
    pinMode(COL_PINS[i], OUTPUT);
    digitalWrite(COL_PINS[i], LOW);
  }
  
  // Debug output
  Serial.println("MAZE GAME READY");
  Serial.println("Player starts at 1,1");
  Serial.println("Exit is at 6,6");
  
  // Quick test pattern to verify display
  testPattern();
}

void loop() {
  // 1. Handle player blinking
  if (millis() - lastBlinkTime > 300) {
    lastBlinkTime = millis();
    playerVisible = !playerVisible;
  }
  
  // 2. Check for button presses (if enough time has passed)
  if (millis() - lastMoveTime > 200) {
    checkButtons();
  }
  
  // 3. Check for serial commands
  checkSerial();
  
  // 4. Display the maze
  displayMaze();
}

// Modified LED matrix display with stronger borders
void displayMaze() {
  // Reset all rows and columns first
  allLedsOff();
  
  // We'll scan each row multiple times to increase brightness
  for (int scan = 0; scan < 2; scan++) {
    for (int row = 0; row < 8; row++) {
      // Turn off all columns first to avoid ghosting
      for (int col = 0; col < 8; col++) {
        digitalWrite(COL_PINS[col], LOW);
      }
      
      // Enable the current row
      digitalWrite(ROW_PINS[row], LOW);
      
      // Turn on LEDs in this row
      for (int col = 0; col < 8; col++) {
        boolean ledOn = false;
        
        // Give priority to player and exit
        if (row == playerY && col == playerX && playerVisible) {
          // Player (blinking)
          ledOn = true;
        }
        else if (row == exitY && col == exitX) {
          // Exit
          ledOn = true;
        }
        else if (maze[row][col] == 1) {
          // Wall
          ledOn = true;
        }
        
        // Light up this LED if needed
        if (ledOn) {
          digitalWrite(COL_PINS[col], HIGH);
        }
      }
      
      // Hold this row pattern visible longer for borders (row 0 and 7)
      if (row == 0 || row == 7) {
        delay(3); // Longer delay for top/bottom borders
      } else {
        delay(2); // Standard delay for other rows
      }
      
      // Disable the row
      digitalWrite(ROW_PINS[row], HIGH);
    }
  }
}

void allLedsOff() {
  // Turn off all rows and columns
  for (int i = 0; i < 8; i++) {
    digitalWrite(ROW_PINS[i], HIGH);  // Rows off
    digitalWrite(COL_PINS[i], LOW);   // Columns off
  }
}

void checkButtons() {
  // Temporarily change column pins to inputs with pull-ups
  for (int i = 4; i < 8; i++) {  // Pins used for buttons are last 4 in COL_PINS
    pinMode(COL_PINS[i], INPUT_PULLUP);
  }
  
  // Small delay to stabilize
  delayMicroseconds(50);
  
  // Check each button
  if (digitalRead(BTN_UP) == LOW) {
    movePlayer(0, -1);
    Serial.println("UP button pressed");
  }
  else if (digitalRead(BTN_RIGHT) == LOW) {
    movePlayer(1, 0);
    Serial.println("RIGHT button pressed");
  }
  else if (digitalRead(BTN_DOWN) == LOW) {
    movePlayer(0, 1);
    Serial.println("DOWN button pressed");
  }
  else if (digitalRead(BTN_LEFT) == LOW) {
    movePlayer(-1, 0);
    Serial.println("LEFT button pressed");
  }
  
  // Change pins back to outputs
  for (int i = 4; i < 8; i++) {
    pinMode(COL_PINS[i], OUTPUT);
    digitalWrite(COL_PINS[i], LOW);
  }
  
  // Update move time
  lastMoveTime = millis();
}

void movePlayer(int dx, int dy) {
  // Calculate new position
  int newX = playerX + dx;
  int newY = playerY + dy;
  
  // Check boundaries and walls
  if (newX < 0 || newX > 7 || newY < 0 || newY > 7 || maze[newY][newX] == 1) {
    // Invalid move - hit wall or boundary
    return;
  }
  
  // Valid move - update player position
  playerX = newX;
  playerY = newY;
  
  // Print position for debugging
  Serial.print("Player at: ");
  Serial.print(playerX);
  Serial.print(",");
  Serial.println(playerY);
  
  // Check if player reached exit
  if (playerX == exitX && playerY == exitY) {
    winAnimation();
    playerX = 1;
    playerY = 1;
  }
}

// Run a test pattern to verify LED matrix is working
void testPattern() {
  // First flash all LEDs on/off
  for (int i = 0; i < 3; i++) {
    // All LEDs on
    for (int row = 0; row < 8; row++) {
      digitalWrite(ROW_PINS[row], LOW);
    }
    for (int col = 0; col < 8; col++) {
      digitalWrite(COL_PINS[col], HIGH);
    }
    delay(200);
    
    // All LEDs off
    allLedsOff();
    delay(200);
  }
  
  // Light up the border to verify all edges work
  for (int i = 0; i < 10; i++) {
    // Top and bottom rows
    digitalWrite(ROW_PINS[0], LOW);
    digitalWrite(ROW_PINS[7], LOW);
    
    // All columns on
    for (int col = 0; col < 8; col++) {
      digitalWrite(COL_PINS[col], HIGH);
    }
    
    delay(100);
    allLedsOff();
    
    // Left and right columns
    for (int row = 0; row < 8; row++) {
      digitalWrite(ROW_PINS[row], LOW);
    }
    digitalWrite(COL_PINS[0], HIGH);
    digitalWrite(COL_PINS[7], HIGH);
    
    delay(100);
    allLedsOff();
  }
}

void winAnimation() {
  // Flash all LEDs
  for (int i = 0; i < 5; i++) {
    // Turn all LEDs on
    for (int row = 0; row < 8; row++) {
      digitalWrite(ROW_PINS[row], LOW);
      for (int col = 0; col < 8; col++) {
        digitalWrite(COL_PINS[col], HIGH);
        delay(5);
        digitalWrite(COL_PINS[col], LOW);
      }
      digitalWrite(ROW_PINS[row], HIGH);
    }
    delay(100);
  }
  
  Serial.println("YOU WIN!");
}

void checkSerial() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd.startsWith("MOVE:")) {
      String direction = cmd.substring(5);
      
      if (direction == "UP") movePlayer(0, -1);
      else if (direction == "RIGHT") movePlayer(1, 0);
      else if (direction == "DOWN") movePlayer(0, 1);
      else if (direction == "LEFT") movePlayer(-1, 0);
      
      Serial.print("Moved ");
      Serial.println(direction);
    }
    else if (cmd.startsWith("POS:")) {
      int commaPos = cmd.indexOf(',');
      if (commaPos > 0) {
        int x = cmd.substring(4, commaPos).toInt();
        int y = cmd.substring(commaPos + 1).toInt();
        
        // Force position update if valid
        if (x >= 0 && x < 8 && y >= 0 && y < 8 && maze[y][x] != 1) {
          playerX = x;
          playerY = y;
          Serial.print("Position set to: ");
          Serial.print(playerX);
          Serial.print(",");
          Serial.println(playerY);
        }
      }
    }
  }
}
