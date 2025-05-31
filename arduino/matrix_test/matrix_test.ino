/*
 * Ultra-Simple LED Matrix Test
 * This sketch tries multiple display methods to find what works with your hardware
 */

// Define pins
const byte ROW_PINS[8] = {6, 7, 8, 9, 10, 11, 12, 13};
const byte COL_PINS[8] = {A3, A2, A1, A0, 5, 4, 3, 2};

// Test mode
int testMode = 0;  // We'll cycle through different test patterns

void setup() {
  Serial.begin(9600);
  
  // All pins as outputs
  for(int i = 0; i < 8; i++) {
    pinMode(ROW_PINS[i], OUTPUT);
    pinMode(COL_PINS[i], OUTPUT);
  }
  
  Serial.println("LED MATRIX TEST");
  Serial.println("Press any key to cycle through test modes");
}

void loop() {
  // Check for serial input to change test mode
  if (Serial.available()) {
    Serial.read(); // Clear the input
    testMode = (testMode + 1) % 6;
    Serial.print("Switched to test mode: ");
    Serial.println(testMode);
  }
  
  // Run the selected test
  switch(testMode) {
    case 0:
      testSingleLED();
      break;
    case 1:
      testFullOn();
      break;
    case 2:
      testBorders();
      break;
    case 3:
      testRowScan();
      break;
    case 4:
      testColumnScan();
      break;
    case 5:
      testInvertedPolarity();
      break;
  }
}

// Test 0: Light up a single LED at position 1,1
void testSingleLED() {
  // Clear everything first
  allOff();
  
  // Try to light up one LED at position 1,1
  // Method 1: Common Anode (ROW LOW, COL HIGH)
  digitalWrite(ROW_PINS[1], LOW);  // Row 1
  digitalWrite(COL_PINS[1], HIGH); // Column 1
  delay(500);
  
  allOff();
  delay(500);
}

// Test 1: Try to turn all LEDs on
void testFullOn() {
  // Method 1: All ROWs LOW, all COLs HIGH (common anode)
  for(int i = 0; i < 8; i++) {
    digitalWrite(ROW_PINS[i], LOW);
    digitalWrite(COL_PINS[i], HIGH);
  }
  delay(1000);
  
  allOff();
  delay(250);
}

// Test 2: Light up just the borders
void testBorders() {
  allOff();
  
  // Top row
  digitalWrite(ROW_PINS[0], LOW);
  for(int col = 0; col < 8; col++) {
    digitalWrite(COL_PINS[col], HIGH);
  }
  delay(200);
  
  allOff();
  
  // Bottom row
  digitalWrite(ROW_PINS[7], LOW);
  for(int col = 0; col < 8; col++) {
    digitalWrite(COL_PINS[col], HIGH);
  }
  delay(200);
  
  allOff();
  
  // Left column
  digitalWrite(COL_PINS[0], HIGH);
  for(int row = 0; row < 8; row++) {
    digitalWrite(ROW_PINS[row], LOW);
  }
  delay(200);
  
  allOff();
  
  // Right column
  digitalWrite(COL_PINS[7], HIGH);
  for(int row = 0; row < 8; row++) {
    digitalWrite(ROW_PINS[row], LOW);
  }
  delay(200);
  
  allOff();
  delay(200);
}

// Test 3: Row scanning
void testRowScan() {
  // Scan through each row
  for(int row = 0; row < 8; row++) {
    // Turn off previous
    allOff();
    
    // Light up this entire row
    digitalWrite(ROW_PINS[row], LOW);
    for(int col = 0; col < 8; col++) {
      digitalWrite(COL_PINS[col], HIGH);
    }
    
    delay(100);
  }
}

// Test 4: Column scanning
void testColumnScan() {
  // Scan through each column
  for(int col = 0; col < 8; col++) {
    // Turn off previous
    allOff();
    
    // Light up this entire column
    digitalWrite(COL_PINS[col], HIGH);
    for(int row = 0; row < 8; row++) {
      digitalWrite(ROW_PINS[row], LOW);
    }
    
    delay(100);
  }
}

// Test 5: Try inverted polarity (for common cathode matrix)
void testInvertedPolarity() {
  // Method: All ROWs HIGH, all COLs LOW (common cathode)
  for(int i = 0; i < 8; i++) {
    digitalWrite(ROW_PINS[i], HIGH);
    digitalWrite(COL_PINS[i], LOW);
  }
  delay(1000);
  
  // All off
  for(int i = 0; i < 8; i++) {
    digitalWrite(ROW_PINS[i], LOW);
    digitalWrite(COL_PINS[i], HIGH);
  }
  delay(250);
}

// Turn everything off
void allOff() {
  // For common anode: ROW HIGH, COL LOW
  for(int i = 0; i < 8; i++) {
    digitalWrite(ROW_PINS[i], HIGH);
    digitalWrite(COL_PINS[i], LOW);
  }
}
