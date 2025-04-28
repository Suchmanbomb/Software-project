#include <Arduino.h>
#include <esp_task_wdt.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc_cal.h"
#include <SPI.h>
#include "pin_config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <time.h>


// Remember to remove these before commiting in GitHub
String ssid = "BTH_Guest";
String password = "Renault19X-15";

// "tft" is the graphics libary, which has functions to draw on the screen
TFT_eSPI tft = TFT_eSPI();

// Display dimentions
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 170

WiFiClient wifi_client;

/**
 * Setup function
 * This function is called once when the program starts to initialize the program
 * and set up the hardware.
 * Carefull when modifying this function.
 */
void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  // Wait for the Serial port to be ready
  while (!Serial);
  Serial.println("Starting ESP32 program...");
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);

  // Connect to WIFI
  WiFi.begin(ssid, password);

  // Will be stuck here until a proper wifi is configured
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Connecting to WiFi...", 10, 10);
    Serial.println("Attempting to connect to WiFi...");
  }

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Connected to WiFi", 10, 10);
  Serial.println("Connected to WiFi");
  // Add your code here 
  
}

/**
 * This is the main loop function that runs continuously after setup.
 * Add your code here to perform tasks repeatedly.
 */
void loop() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Team 5, version 1.0", 10, 10);

  delay(3000);
  HTTPClient http;
  String smhiUrl = "https://opendata-download-metanalys.smhi.se/api/category/mesan2g/version/1/geotype/point/lon/16/lat/58/data.json";

  http.begin(smhiUrl);
  int httpCode = http.GET(); //HTTP-calll to SMHI 

  if (httpCode == HTTP_CODE_OK) {
    auto responseText = http.getString();
    DynamicJsonDocument weatherData(16384); //document to store JSON_data
    DeserializationError error = deserializeJson(weatherData, responseText);

    if (!error) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(5, 5);

      JsonObject forecast = weatherData["timeSeries"][0];
      JsonArray params = forecast["parameters"];
      //loop through all data, stores under different parameters 
      float temperature = 0.0;
      float wind = 0.0;
      float humidity = 0.0;

      for (JsonObject item : params) {
        String paramName = item["name"];
        float value = item["values"][0];

        if (paramName == "t")      temperature = value;
        else if (paramName == "ws") wind = value;
        else if (paramName == "r")  humidity = value;
      }
      //shows wheater on the screen 
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextSize(2);
      tft.println("Karlskrona");
      tft.println();
      tft.printf("Temp: %.1f Celsius\n", temperature);
      tft.printf("Wind: %.1f m/s\n", wind);
      tft.printf("Humidity: %.0f%%\n", humidity);
    } else {
      tft.fillScreen(TFT_RED);
      tft.setCursor(10, 10);
      tft.setTextColor(TFT_WHITE, TFT_RED);
      tft.setTextSize(2);
      tft.println("JSON parse error!");
    }
  } else {
    tft.fillScreen(TFT_RED);
    tft.setCursor(10, 10);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setTextSize(2);
    tft.println("HTTP error!");
  }

  http.end(); // Avslutar HTTP-förbindelsen
  delay(10000); // Vänta 10 sekunder innan nästa uppdatering
}