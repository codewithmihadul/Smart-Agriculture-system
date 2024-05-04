#define BLYNK_TEMPLATE_ID "TMPL6zMU5pFmc"
#define BLYNK_TEMPLATE_NAME "Agriculture Project"
#define BLYNK_AUTH_TOKEN "Bap9WDoDGcVojYWWzEsSRxXO3n6Ymk8R"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"
#include <FastLED.h>
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wifi ";
char pass[] = "Wifi Password";

#define LED_PIN     21
#define BRIGHTNESS  255
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
 
const uint8_t kMatrixWidth  = 16;
const uint8_t kMatrixHeight = 16;
const bool    kMatrixSerpentineLayout = true; 

#define DHTPIN 4
#define DHTTYPE DHT22
 
DHT dht(DHTPIN, DHTTYPE);
const int soilMoisturePin = 35; // Analog pin for the soil moisture sensor
const int waterLevelPin = 34;   // Analog pin for the water level sensor
const int ledPin = 2;

// Constants for soil moisture thresholds
const int SOIL_MOISTURE_LOW = 50;  // Adjusted to match the 0-100 range
const int SOIL_MOISTURE_HIGH = 50; // Adjusted to match the 0-100 range
 
const int pumpPin = 5;  // Change this to the actual pin connected to the DC pump on ESP32
int pumpState = LOW;    // Initialize pumpState to LOW (off)
int pumpMode = 0;       // 0 for automatic, 1 for manual
 
int waterLevel = 0;     // Water level indicator value
unsigned long previousMillis = 0;
const unsigned long interval = 1000; // Interval in milliseconds

void setup() {
  Serial.begin(115200);
  digitalWrite(pumpPin,LOW);
  pinMode(ledPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  dht.begin();
  Blynk.begin(auth, ssid, pass);
  Blynk.virtualWrite(V2, LOW);
  Blynk.virtualWrite(V3, "LED OFF");
  Blynk.virtualWrite(V6, LOW); // Initialize the mode button to automatic mode
  Blynk.virtualWrite(V5, LOW); // Initialize the pump button to OFF and On
  Blynk.virtualWrite(V8, LOW); // Initialize the RGB Led Strip button to OFF And On
  delay(3000);
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,80);
  LEDS.setBrightness(BRIGHTNESS);
} 

BLYNK_WRITE(V5) { // This function is called when the pump button state changes (Manual Control)
  if (pumpMode == 1) { // Check if the mode is manual
    pumpState = param.asInt(); // Read the state of the pump button (0 for OFF, 1 for ON)
    digitalWrite(pumpPin, pumpState); // Turn the pump on or off based on button state
  }
}
 
BLYNK_WRITE(V6) { // This function is called when the mode button state changes
  pumpMode = param.asInt(); // Read the state of the mode button (0 for automatic, 1 for manual)
}
 
BLYNK_WRITE(V8) {
  int value = param.asInt();
  if (value == 1) {
    // Turn on the LED strip and start the animation
    ledStripOn = true;
  } else {
    // Turn off the LED strip
    ledStripOn = false;
    // You can also add code to clear the LED strip here if needed
  }
}

void loop() {
  Blynk.run();
  timer.run();
 
  updateLEDStrip();
  unsigned long currentMillis = millis();
 
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
 
    // Read soil moisture or water level based on the flag
    if (pumpMode == 0) {
      int soilSensorValue = analogRead(soilMoisturePin);
      Serial.println(soilSensorValue);
      mappedSoilMoisture = map(soilSensorValue, 1023, 2750, 100, 0); // Update mappedSoilMoisture
      autoControlPump(mappedSoilMoisture); // Automatically control the pump based on soil moisture if in automatic mode
    }

  // Read water level
    waterLevel = analogRead(waterLevelPin);
    Serial.println(waterLevel);
    if (waterLevel < 450) {
      waterLevel = 0; // Set water level to 0 if below a threshold
    } else {
      waterLevel = map(waterLevel, 1200, 1600, 0, 100); // Map water level to 0-100
    }

    // Control LED
    controlLED(mappedSoilMoisture);
 
    // Read and send sensor data
    readAndSendSensorData();
    Blynk.virtualWrite(V7, waterLevel); // Send water level to Blynk
  }
}


