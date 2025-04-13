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
String ssid = "RojFagerberg";
String password = "ostergard2";

// "tft" is the graphics libary, which has functions to draw on the screen
TFT_eSPI tft = TFT_eSPI();

// Display dimentions
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 170

WiFiClient wifi_client;
int displayMode = 0;

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
  int button1 = digitalRead(PIN_BUTTON_1);
int button2 = digitalRead(PIN_BUTTON_2);

if (button1 == LOW) {
  displayMode = 0;
  delay(300); // debounce
} else if (button2 == LOW) {
  displayMode = 1;
  delay(300); // debounce
}
if (displayMode == 1) {
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
      tft.printf("Temp: %.1f C\n", temperature);
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
else if (displayMode == 0) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Team 5", 10, 10);
  tft.drawString("Version 1.0", 10, 40);
  delay(3000);
}

}


// TFT Pin check
  //////////////////
 // DO NOT TOUCH //
//////////////////
#if PIN_LCD_WR  != TFT_WR || \
    PIN_LCD_RD  != TFT_RD || \
    PIN_LCD_CS    != TFT_CS   || \
    PIN_LCD_DC    != TFT_DC   || \
    PIN_LCD_RES   != TFT_RST  || \
    PIN_LCD_D0   != TFT_D0  || \
    PIN_LCD_D1   != TFT_D1  || \
    PIN_LCD_D2   != TFT_D2  || \
    PIN_LCD_D3   != TFT_D3  || \
    PIN_LCD_D4   != TFT_D4  || \
    PIN_LCD_D5   != TFT_D5  || \
    PIN_LCD_D6   != TFT_D6  || \
    PIN_LCD_D7   != TFT_D7  || \
    PIN_LCD_BL   != TFT_BL  || \
    TFT_BACKLIGHT_ON   != HIGH  || \
    170   != TFT_WIDTH  || \
    320   != TFT_HEIGHT
#error  "Error! Please make sure <User_Setups/Setup206_LilyGo_T_Display_S3.h> is selected in <TFT_eSPI/User_Setup_Select.h>"
#error  "Error! Please make sure <User_Setups/Setup206_LilyGo_T_Display_S3.h> is selected in <TFT_eSPI/User_Setup_Select.h>"
#error  "Error! Please make sure <User_Setups/Setup206_LilyGo_T_Display_S3.h> is selected in <TFT_eSPI/User_Setup_Select.h>"
#error  "Error! Please make sure <User_Setups/Setup206_LilyGo_T_Display_S3.h> is selected in <TFT_eSPI/User_Setup_Select.h>"
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
#error  "The current version is not supported for the time being, please use a version below Arduino ESP32 3.0"
#endif