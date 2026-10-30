# shuvabihani


#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

// --- Component Pin Definitions (XIAO ESP32-C3) ---
#define TFT_SCLK D9
#define TFT_MOSI D10
#define TFT_DC   D4
#define TFT_CS   D5
#define TFT_RST  -1  // Hardwired to 3.3V in schematic

#define I2C_SDA  D6
#define I2C_SCL  D8

#define SW1 D3
#define SW2 D2
#define SW3 D1
#define SW4 D0

#define BUZZER_PIN D7

// --- Hardware Objects ---
Adafruit_BME680 bme; // BME680 over I2C

// Fix setColRowStart() by exposing it via a subclass (from reference)
class MyST7789 : public Adafruit_ST7789 {
public:
  MyST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst)
    : Adafruit_ST7789(cs, dc, mosi, sclk, rst) {}
  void setOffsets(uint8_t col, uint8_t row) {
    _colstart = _colstart2 = col;
    _rowstart = _rowstart2 = row;
  }
};

MyST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void setup() {
  Serial.begin(115200);
  
  // 1. Initialize Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // 2. Initialize MX Switches with Internal Pull-ups
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);

  // 3. Initialize TFT Display
  // Note: Backlight (BL) is hardwired to 3.3V, so it turns on automatically
  tft.init(76, 284);             // Base panel size from reference
  tft.setOffsets(82, 18);        // Offsets for specific screen boundaries
  tft.invertDisplay(false);      // Flipped colors configuration
  tft.setRotation(1);            // Landscape mode
  tft.fillScreen(ST77XX_BLACK);
  Serial.println("TFT Initialized!");

  // 4. Initialize I2C and BME680
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // 0x76 is the standard I2C address for the BME680 when SDO is tied to GND
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("BME680 Error!");
    while (1); 
  }

  // Set up oversampling and filter initialization for BME680
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320°C for 150 ms
}

void loop() {
  // Read sensor data
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  // Draw UI Background (only overwrite text areas to prevent flickering)
  tft.fillRect(0, 0, 240, 135, ST77XX_BLACK); 
  
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);

  // Display Temperature
  tft.setCursor(10, 10);
  tft.print("Temp: ");
  tft.print(bme.temperature);
  tft.print(" C");

  // Display Humidity
  tft.setCursor(10, 40);
  tft.print("Humidity: ");
  tft.print(bme.humidity);
  tft.print(" %");

  // Display Gas Resistance (VOCs)
  tft.setCursor(10, 70);
  tft.print("VOCs: ");
  tft.print(bme.gas_resistance / 1000.0);
  tft.print(" kOhm");

  // Input Handling: If SW4 is pressed, sound the buzzer and show alert
  if (digitalRead(SW4) == LOW) {
    digitalWrite(BUZZER_PIN, HIGH);
    
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 100);
    tft.print("ALARM TRIGGERED!");
    
    delay(200); // Debounce / Buzzer active time
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Add more button actions for SW1, SW2, SW3 here as needed

  delay(2000); // Wait 2 seconds between environmental reads
}