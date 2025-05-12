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

// WiFi credentials
String ssid = "BTH_Guest";
String password = "Renault19X-15";

// Display
TFT_eSPI tft = TFT_eSPI();

// Display dimensions
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 170

WiFiClient wifi_client;

// Button debounce
#define BUTTON_DEBOUNCE_DELAY 200
unsigned long lastButton1Press = 0;
unsigned long lastButton2Press = 0;

// För hålla-tid vid tvåknappskombination
unsigned long bothButtonsPressedStart = 0;
bool bothButtonsHeld = false;

// Skärm-states
enum ScreenState {
  BOOT,
  MENU,
  FORECAST,
  HISTORICAL,
  SETTINGS
};

ScreenState currentScreen = BOOT;

// Menu navigation
int menuIndex = 0; // 0 = Forecast, 1 = Historical, 2 = Settings
const int menuItemCount = 3;

// Funktioner
void showBootScreen();
void showMenu();
void showForecastScreen();
void showHistoricalScreen();
void showSettingsScreen();
void checkButtons();
void drawMenuWithHighlight(int index);
void showLoadingScreen(String text);

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Starting ESP32 program...");

  // Initialize display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Button setup
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);

  // Connect to WIFI
  WiFi.begin(ssid, password);

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

  // Show boot screen for 3 seconds
  showBootScreen();
  delay(3000);

  // Move to menu
  currentScreen = MENU;
  showMenu();
}

void loop() {
  checkButtons();

  if (currentScreen == FORECAST) {
    showForecastScreen();
    delay(10000); // Vänta innan nästa uppdatering
    currentScreen = MENU;
    showMenu();
  }
}

void checkButtons() {
  bool button1Pressed = digitalRead(PIN_BUTTON_1) == LOW;
  bool button2Pressed = digitalRead(PIN_BUTTON_2) == LOW;
  unsigned long now = millis();

  // Om båda knappar hålls intryckta
  if (button1Pressed && button2Pressed) {
    if (!bothButtonsHeld) {
      bothButtonsPressedStart = now;
      bothButtonsHeld = true;
    }
    if (now - bothButtonsPressedStart > 1000) { // håll i minst 1 sekund
      currentScreen = MENU;
      menuIndex = 0;
      showMenu();
      bothButtonsHeld = false;
      delay(500); // skydda mot studs
    }
    return;
  } else {
    bothButtonsHeld = false;
  }

  if (button1Pressed && (now - lastButton1Press > BUTTON_DEBOUNCE_DELAY)) {
    lastButton1Press = now;

    if (currentScreen == MENU) {
      menuIndex = (menuIndex + 1) % menuItemCount;
      drawMenuWithHighlight(menuIndex);
    }
  }

  if (button2Pressed && (now - lastButton2Press > BUTTON_DEBOUNCE_DELAY)) {
    lastButton2Press = now;

    if (currentScreen == MENU) {
      showLoadingScreen("Loading...");
      switch (menuIndex) {
        case 0:
          currentScreen = FORECAST;
          break;
        case 1:
          currentScreen = HISTORICAL;
          showHistoricalScreen();
          break;
        case 2:
          currentScreen = SETTINGS;
          showSettingsScreen();
          break;
      }
    }
  }
}

// BOOT SCREEN
void showBootScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Weather Project", 40, 40);
  tft.drawString("Group 5", 100, 70);
  tft.drawString("v1.0", 130, 100);
}

// MENY
void showMenu() {
  tft.fillScreen(TFT_BLACK);
  drawMenuWithHighlight(menuIndex);
}

void drawMenuWithHighlight(int index) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Main Menu", 90, 20);

  tft.setTextSize(1);

  if (index == 0) tft.setTextColor(TFT_WHITE, TFT_BLACK);
  else tft.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  tft.drawString("> Forecast", 40, 60);

  if (index == 1) tft.setTextColor(TFT_WHITE, TFT_BLACK);
  else tft.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  tft.drawString("> Historical", 40, 80);

  if (index == 2) tft.setTextColor(TFT_WHITE, TFT_BLACK);
  else tft.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  tft.drawString("> Settings", 40, 100);

  tft.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  tft.drawString("Btn1=Next Btn2=Select", 20, 140);
}

// LOADING SCREEN
void showLoadingScreen(String text) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString(text, 100, 80);
  delay(300); // visa kort
}

// FORECAST
void showForecastScreen() {
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

  http.end();
}

// HISTORICAL
void showHistoricalScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Historical Data", 60, 20);

  tft.setTextSize(1);
  tft.drawString("Scroll with buttons", 60, 100);
}

// SETTINGS
void showSettingsScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_PINK, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Settings", 100, 20);

  tft.setTextSize(1);
  tft.drawString("1. Change City", 40, 60);
  tft.drawString("2. Change Parameter", 40, 80);
  tft.drawString("3. Reset Defaults", 40, 100);
}