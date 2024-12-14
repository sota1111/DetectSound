#include <M5Stack.h>

void setup() {
  M5.begin();
  M5.Lcd.println("Starting ADC on PORT B");
}

void loop() {
  // Read ADC value from PORT B (pin G36)
  int adcValue = analogRead(36); // GPIO36 = VP pin

  // Print ADC value on the LCD
  M5.Lcd.setCursor(0, 20); // Move the cursor to avoid overwriting "Starting ADC"
  M5.Lcd.fillRect(0, 20, 320, 20, BLACK); // Clear previous ADC value
  M5.Lcd.printf("ADC Value: %d", adcValue);

  delay(500); // Wait for 500ms
}