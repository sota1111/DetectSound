#include <M5Stack.h>

void setup() {
  M5.begin();
  M5.Lcd.println("Starting ADC on PORT B");
}

void loop() {
  int maxAdcValue = 0; // Variable to store the maximum ADC value

  // Measure ADC values for 500ms (10ms interval)
  for (int i = 0; i < 50; i++) {
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
}
