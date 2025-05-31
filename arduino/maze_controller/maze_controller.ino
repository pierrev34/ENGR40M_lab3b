/*
 * Simple Web Serial API Test
 * For ENGR40M Lab 3B
 */

// Counter for messages
unsigned long lastSent = 0;
int counter = 0;

// LED pin for visual feedback
const int LED_PIN = 13;

void setup() {
  // Set up LED for visual feedback
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize serial at 9600 baud
  Serial.begin(9600);
  
  // Small delay to let things stabilize
  delay(1000);
  
  // Send initial message
  Serial.println("ARDUINO_TEST:STARTED");
}

void loop() {
  // Blink LED to show the Arduino is running
  digitalWrite(LED_PIN, (millis() / 500) % 2); // Blink every 500ms
  
  // Send a message every second
  if (millis() - lastSent > 1000) {
    lastSent = millis();
    Serial.print("TEST_MESSAGE:");
    Serial.println(counter++);
  }
  
  // Echo any received data
  while (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    Serial.print("ECHO:");
    Serial.println(data);
  }
}
    
    // Process player position: P:x,y
    if (command.startsWith("P:")) {
      int commaIndex = command.indexOf(',', 2);
      if (commaIndex > 0) {
        playerX = command.substring(2, commaIndex).toInt();
        playerY = command.substring(commaIndex + 1).toInt();
      }
    }
    
    // Process exit position: E:x,y
    else if (command.startsWith("E:")) {
      int commaIndex = command.indexOf(',', 2);
      if (commaIndex > 0) {
        exitX = command.substring(2, commaIndex).toInt();
        exitY = command.substring(commaIndex + 1).toInt();
      }
    }
    
    // Process maze row data: M:row:data
    else if (command.startsWith("M:")) {
      int firstColon = command.indexOf(':', 2);
      if (firstColon > 0) {
        int row = command.substring(2, firstColon).toInt();
        String rowData = command.substring(firstColon + 1);
        
        // Parse the row data (0 = path, 1 = wall)
        if (row >= 0 && row < 8 && rowData.length() == 8) {
          for (int col = 0; col < 8; col++) {
            maze[row][col] = (rowData.charAt(col) == '1') ? 1 : 0;
          }
        }
      }
    }
  }
}

// Read button states and send movement commands
void readButtons() {
  // Read current button states (LOW = pressed with pull-up resistors)
  bool upState = digitalRead(UP_BTN) == LOW;
  bool downState = digitalRead(DOWN_BTN) == LOW;
  bool leftState = digitalRead(LEFT_BTN) == LOW;
  bool rightState = digitalRead(RIGHT_BTN) == LOW;
  
  // Check for button presses (detect change from not pressed to pressed)
  if (upState && !upPressed) {
    Serial.println("MOVE:UP");
    delay(200); // Debounce
  }
  
  if (downState && !downPressed) {
    Serial.println("MOVE:DOWN");
    delay(200); // Debounce
  }
  
  if (leftState && !leftPressed) {
    Serial.println("MOVE:LEFT");
    delay(200); // Debounce
  }
  
  if (rightState && !rightPressed) {
    Serial.println("MOVE:RIGHT");
    delay(200); // Debounce
  }
  
  // Update button states
  upPressed = upState;
  downPressed = downState;
  leftPressed = leftState;
  rightPressed = rightState;
}
