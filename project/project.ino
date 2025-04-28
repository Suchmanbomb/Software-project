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
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    auto responseText = http.getString();
    DynamicJsonDocument weatherData(16384);
    DeserializationError error = deserializeJson(weatherData, responseText);


    if (!error) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(5, 5);

      JsonArray forecastArray = weatherData["timeSeries"];

      int x = 0;
      int y = 10;
      int displayedHours = 0;

      for (JsonObject forecast : forecastArray) {
        JsonArray params = forecast["parameters"];

        float temperature = 0.0;
        int weatherSymbol = -1;

        for (JsonObject item : params) {
          String paramName = item["name"];
          if (paramName == "t") {
            temperature = item["values"][0];
          }
          else if (paramName == "Wsymb2") {
            weatherSymbol = item["values"][0];
          }
        }

        // Rita ut data
        if (weatherSymbol != -1) {
          drawWeatherSymbol(x + 10, y, weatherSymbol);
          tft.setCursor(x + 10, y + 30);
          tft.setTextColor(TFT_WHITE, TFT_BLACK);
          tft.setTextSize(1);
          tft.printf("%.1fC", temperature);
          
          x += 60; // Nästa kolumn

          if (x > DISPLAY_WIDTH - 60) {
            x = 0;
            y += 60; // Nästa rad
          }
          displayedHours += 1;
        }

        // Begränsa till cirka 24 timmar (ex. varje 1h eller 3h beroende på API)
        if (displayedHours >= 8) break; 
      }
<<<<<<< HEAD
=======
      //shows wheater on the screen 
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextSize(2);
      tft.println("Karlskrona");
      tft.println();
      tft.printf("Temp: %.1f Celsius\n", temperature);
      tft.printf("Wind: %.1f m/s\n", wind);
      tft.printf("Humidity: %.0f%%\n", humidity);
>>>>>>> a200ec379c8a1abdbefcb9a93b235b0f2fe86341
    } else {
      showError("JSON parse error!");
    }
  } else {
    showError("HTTP error!");
  }


  http.end();
  delay(10000);
}