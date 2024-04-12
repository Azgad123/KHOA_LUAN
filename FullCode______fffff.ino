#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Pin configuration for the SPI connection
#define TFT_CS 8   // CS pin
#define TFT_RST 9  // RST pin
#define TFT_DC 10  // DC pin
#define TFT_BL 11  // BLK pin

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define measurePin 26
#define ledPower 22
#define RELAY_1_PIN 6
#define RELAY_2_PIN 7

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 515.98; // Thay đổi giá trị này với nồng độ bụi thực tế của bạn

const float GAS_THRESHOLD = 1;
const float DUST_THRESHOLD = 45;

float gas_concentration = 0;

Adafruit_BME680 bme;

void setup() {
  Serial.begin(9600);
  
  // Khởi tạo kết nối SPI
  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

  // Khởi tạo màn hình TFT
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);

  // Bật đèn nền
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  pinMode(ledPower, OUTPUT);
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);

  // if (!bme.begin()) {
  //   Serial.println("Could not find a valid BME680 sensor, check wiring!");
  //   while (1);
  // }
void loop() {
  digitalWrite(ledPower, LOW);
  delayMicroseconds(samplingTime);
  voMeasured = analogRead(measurePin);
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower, HIGH);
  delayMicroseconds(sleepTime);
  calcVoltage = voMeasured * (3.3 / 1024.0);
  dustDensity = 170 * calcVoltage - 0.1;

  if (bme.performReading()) {
    tft.fillScreen(ST7735_BLACK);

    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.setTextColor(ST7735_WHITE);
    tft.print("AIR");
    tft.setCursor(44, 0);
    tft.print("QUALITY");

    tft.setCursor(0, 35);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_RED);
    tft.print("PM2.5: ");
    tft.setTextColor(ST7735_GREEN);
    tft.setCursor(44, 35);
    tft.print(dustDensity);
    tft.print(" µg/m3");

    tft.setCursor(0, 60);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_RED);
    tft.print("Temp: ");
    tft.setTextColor(ST7735_GREEN);
    tft.setCursor(44, 60);
    tft.print(bme.temperature);
    tft.print(" *C");

    tft.setCursor(0, 85);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_RED);
    tft.print("Press: ");
    tft.setTextColor(ST7735_GREEN);
    tft.setCursor(44, 85);
    tft.print(bme.pressure / 100.0);
    tft.print(" hPa");

    tft.setCursor(0, 110);
    tft.setTextColor(ST7735_RED);
    tft.setTextSize(1);
    tft.print("Humi: ");
    tft.setTextColor(ST7735_GREEN);
    tft.setCursor(44, 110);
    tft.print(bme.humidity);
    tft.print(" %");

    tft.setCursor(0, 135);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_RED);
    tft.print("VOCs: ");
    float gas_resistance = bme.gas_resistance / 1000.0;
    gas_concentration = gas_resistance * 0.01;
    tft.setTextColor(ST7735_GREEN);
    tft.setCursor(44, 135);
    tft.print(gas_concentration);
    tft.print(" µg/m³");

    // Relay 1 control based on gas concentration
    if (gas_concentration > GAS_THRESHOLD) {
      digitalWrite(RELAY_1_PIN, LOW);
      Serial.println("Relay 1 ON");
    } else {
      digitalWrite(RELAY_1_PIN, HIGH);
      Serial.println("Relay 1 OFF");
    }
  } else {
    Serial.println("Failed to perform BME680 reading");
  }

  // Relay 2 control based on dust density
  if (dustDensity > DUST_THRESHOLD) {
    digitalWrite(RELAY_2_PIN, LOW);
    Serial.println("Relay 2 ON");
  } else {
    digitalWrite(RELAY_2_PIN, HIGH);
    Serial.println("Relay 2 OFF");
  }

  Serial.print("Voltage: ");
  Serial.print(calcVoltage);
  Serial.print("V, Dust Density: ");
  Serial.print(dustDensity);
  Serial.println(" µg/m3");

  // Check if both conditions are met
  if (gas_concentration > GAS_THRESHOLD && dustDensity > DUST_THRESHOLD) {
    // Perform an action or print a message
    Serial.println("Both conditions met: Gas > 0.8 and Dust Density > 10");
  }

  delay(5000);
}