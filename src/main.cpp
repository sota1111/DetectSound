#include <M5Stack.h>

void setup() {
  M5.begin();
  M5.Lcd.println("Starting ADC on PORT B");
}

void loop() {
  int maxAdcValue = 0; // Variable to store the maximum ADC value

  // Measure ADC values for 1000ms (10ms interval)
  for (int i = 0; i < 100; i++) {
    int adcValue = analogRead(36); // GPIO36 = VP pin
    if (adcValue > maxAdcValue) {
      maxAdcValue = adcValue;
    }
    delay(10); // Wait for 10ms
  }

  // Print the maximum ADC value on the LCD
  M5.Lcd.setCursor(0, 20); // Move the cursor to avoid overwriting "Starting ADC"
  M5.Lcd.fillRect(0, 20, 320, 20, BLACK); // Clear previous ADC value
  M5.Lcd.printf("Max ADC Value: %d", maxAdcValue);

  // Check if maxAdcValue exceeds 2100
  if (maxAdcValue > 2100) {
    M5.Lcd.fillScreen(BLACK); // Clear the screen
    M5.Lcd.setTextSize(4);    // Set large text size
    M5.Lcd.setTextColor(RED); // Set text color to red
    M5.Lcd.setCursor(40, 100);
    M5.Lcd.println("Big Sound");
    delay(1000); // Wait for 1 second before continuing
    M5.Lcd.fillScreen(BLACK); // Clear the screen again
    M5.Lcd.setTextSize(2);    // Reset text size
    M5.Lcd.setTextColor(WHITE); // Reset text color
  }
}
