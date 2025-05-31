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
