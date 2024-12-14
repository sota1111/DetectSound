#include <M5Stack.h>

#define THRESHOLD 2200

void setup() {
  M5.begin();
  M5.Lcd.println("Starting ADC on PORT B");
  M5.Lcd.fillScreen(BLACK); // Clear the screen again
  M5.Lcd.setTextSize(3);    // Reset text size
  M5.Lcd.setTextColor(WHITE); // Reset text color
}

void loop() {
  int adcValue;
  int countOverThreshold = 0; // Counter for values over the threshold

  // Measure ADC values for 1000ms (10ms interval)
  for (int i = 0; i < 100; i++) {
    adcValue = analogRead(36); // GPIO36 = VP pin
    if (adcValue > THRESHOLD) {
      countOverThreshold++;
    }
    delay(10); // Wait for 10ms
  }

  // Print the count of values over the threshold on the LCD
  M5.Lcd.setCursor(0, 20); // Move the cursor to avoid overwriting "Starting ADC"
  M5.Lcd.fillRect(0, 20, 320, 20, BLACK); // Clear previous text
  M5.Lcd.printf("adcValue: %d\n", adcValue);
  M5.Lcd.printf("Count > %d: %d", THRESHOLD, countOverThreshold);

  // Check if countOverThreshold exceeds 10
  if (countOverThreshold >= 10) {
    M5.Lcd.fillScreen(BLACK); // Clear the screen
    M5.Lcd.setTextSize(4);    // Set large text size
    M5.Lcd.setTextColor(RED); // Set text color to red
    M5.Lcd.setCursor(40, 100);
    M5.Lcd.println("Big Sound");
    delay(1000); // Wait for 1 second before continuing
    M5.Lcd.fillScreen(BLACK); // Clear the screen again
    M5.Lcd.setTextSize(3);    // Reset text size
    M5.Lcd.setTextColor(WHITE); // Reset text color
  }
}
