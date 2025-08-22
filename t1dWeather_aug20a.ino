#include "arduino_secrets.h"
// t1dWeather
// Author:      Tom Crites
// Version:     1.0.0
// Create Date: 8/22/2025
// Description: This is a home weather station built with...
//              Arduino R4 Wifi
//              BME280 - Temperature/Humidiy/Pressure Sensor
//              DS1302 - Real Time Clock
//              GC9A01 - 1.28 Round TFT Display
//              
//              Wiring in the readme.adoc
//              
// Modified:    xx/xx/xxxx
// Version:     x.x.x
// Description: xxxxxxxxxxxxxxxxxxxx
//
//       1    1    2    2    3    3    4    4    5    5    6    6    7
//..5....0....5....0....5....0....5....0....5....0....5....0....5....0

// BME280

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme; 

// DS1302 - Real Time Clock

#include <RtcDS1302.h>
#include <ThreeWire.h>

const int IO = 4;    // DAT Yellow
const int SCLK = 5;  // CLK Purple
const int CE = 2;    // RST White

ThreeWire myWire(4, 5, 2);  // IO, SCLK, CE

RtcDS1302<ThreeWire> rtc(myWire);

String dayOfWeek[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", 
"Friday", "Saturday"};

// GC9A01

#include <DIYables_TFT_Round.h>
#include <SPI.h>

#define PIN_CS     10
#define PIN_DC     9
#define PIN_RST    8

#define COLOR_BLACK DIYables_TFT::colorRGB(0, 0, 0)  // Black
#define COLOR_WHITE DIYables_TFT::colorRGB(255, 255, 255)  // Black
#define COLOR_RED DIYables_TFT::colorRGB(180, 35, 35)
#define COLOR_GREEN DIYables_TFT::colorRGB(0, 255, 125)
#define COLOR_BLUE DIYables_TFT::colorRGB(0, 190, 255)

#define FONT_4_DIGITS 7
#define Y_4_DIGITS 35
#define X_4_DIGITS 90

#define FONT_3_DIGITS 11
#define Y_3_DIGITS 25
#define X_3_DIGITS 85

#define FONT_2_DIGITS 15
#define Y_2_DIGITS 40
#define X_2_DIGITS 65

#define FONT_1_DIGITS 18
#define Y_1_DIGITS 80
#define X_1_DIGITS 55

DIYables_TFT_GC9A01_Round TFT_display(PIN_RST, PIN_DC, PIN_CS);

// Sketch

#include "thingProperties.h"

unsigned long previousMillis = 0;
const long interval = 2000;

void setup() {

  // Serial

  Serial.begin(9600);
  while (!Serial);

  // Defined in thingProperties.h
  
  initProperties();

  // Connect to Arduino IoT Cloud
  
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  */

  // DS1302 - Real Time Clock

  rtc.Begin();

  // BME280

  if (!bme.begin(0x76)) {  // Try 0x76 or 0x77 depending on your module
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println("BME280 sensor initialized.");

  // GC9A01
  
  TFT_display.begin();
  
}

void loop() {
  
  ArduinoCloud.update();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis;

    location = "Steubenville, OH";
    
    // DS1302 - Real Time Clock
    
    RtcDateTime now = rtc.GetDateTime();

    lastUpdatedDOW = dayOfWeek[now.DayOfWeek()].c_str();
  
    char bufferDateTime[40];
  
    sprintf(bufferDateTime, "%04u-%02u-%02u %02u:%02u:%02u",
              now.Year(), now.Month(), now.Day(),
             now.Hour(), now.Minute(), now.Second());
  
    lastUpdated = String(bufferDateTime);
  
    Serial.print("Last Updated: ");
    Serial.println(lastUpdated);

    // BME280

    float tempC = bme.readTemperature();
    tempF = (tempC * 9.0 / 5.0) + 32.0;
    humidity = bme.readHumidity();    
    heatIndex = computeHeatIndex(tempF, humidity);
    float dewPointC = calculateDewPoint(tempC, humidity); 
    dewPointF = (dewPointC * 9.0 / 5.0) + 32.0;
    float pressure_hPa = bme.readPressure() / 100.0F; 
    pressure_inHg = pressure_hPa * 0.02953;

    // GC9A01
    
    int roundedTemp = round(tempF);

    if(roundedTemp < -9){

    TFT_display.fillScreen(COLOR_BLACK);
    TFT_display.setTextColor(COLOR_BLUE);
    TFT_display.setTextSize(FONT_3_DIGITS); 
    TFT_display.setCursor(Y_3_DIGITS, X_3_DIGITS);

  }

  if(roundedTemp < 0 && roundedTemp > -10){

    TFT_display.fillScreen(COLOR_BLACK);
    TFT_display.setTextColor(COLOR_BLUE);
    TFT_display.setTextSize(FONT_2_DIGITS); 
    TFT_display.setCursor(Y_2_DIGITS, X_2_DIGITS);

  }
  if(roundedTemp > -1 && roundedTemp < 10){

    TFT_display.fillScreen(COLOR_BLACK);
    TFT_display.setTextColor(COLOR_BLUE);
    TFT_display.setTextSize(FONT_1_DIGITS); 
    TFT_display.setCursor(Y_1_DIGITS, X_1_DIGITS);

  }

  if(roundedTemp > 10 && roundedTemp < 60){

    TFT_display.fillScreen(COLOR_BLACK);
    TFT_display.setTextColor(COLOR_BLUE);
    TFT_display.setTextSize(FONT_2_DIGITS); 
    TFT_display.setCursor(Y_2_DIGITS, X_2_DIGITS);

  }

  if(roundedTemp > 59 && roundedTemp < 81){

    TFT_display.fillScreen(COLOR_BLACK);
    TFT_display.setTextColor(COLOR_GREEN);
    TFT_display.setTextSize(FONT_2_DIGITS); 
    TFT_display.setCursor(Y_2_DIGITS, X_2_DIGITS);

  }
  if(roundedTemp > 80 && roundedTemp < 100){

    TFT_display.fillScreen(COLOR_BLACK);
    TFT_display.setTextColor(COLOR_RED);
    TFT_display.setTextSize(FONT_2_DIGITS); 
    TFT_display.setCursor(Y_2_DIGITS, X_2_DIGITS);

  }

  if(roundedTemp > 99){

    TFT_display.fillScreen(COLOR_BLACK);
    TFT_display.setTextColor(COLOR_RED);
    TFT_display.setTextSize(FONT_3_DIGITS); 
    TFT_display.setCursor(Y_3_DIGITS, X_3_DIGITS);

  }

  TFT_display.print(roundedTemp);

    Serial.print("Temp: ");
    Serial.print(tempC);
    Serial.println(" °C,");
    Serial.print("Temp: ");
    Serial.print(tempF);
    Serial.println(" °F,");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Pressure: ");
    Serial.print(pressure_hPa);
    Serial.println(" hPa");
    Serial.print("Pressure: ");
    Serial.print(pressure_inHg);
    Serial.println(" inHg");
    Serial.print("Heat Index: ");
    Serial.print(heatIndex);
    Serial.println(" °F,");
    Serial.print("Dew Point: ");
    Serial.print(dewPointC);
    Serial.println(" °C,");
    Serial.print("Dew Point: ");
    Serial.print(dewPointF);
    Serial.println(" °F,");

  }

  delay(1000);
  
}

float computeHeatIndex(float tempF, float humidity) {
  
  float HI = -42.379 + 2.04901523 * tempF + 10.14333127 * humidity
             - 0.22475541 * tempF * humidity - 0.00683783 * tempF * tempF
             - 0.05481717 * humidity * humidity
             + 0.00122874 * tempF * tempF * humidity
             + 0.00085282 * tempF * humidity * humidity
             - 0.00000199 * tempF * tempF * humidity * humidity;
  return HI;
  
}

float calculateDewPoint(float tempC, float humidity) {
  
  float a = 17.27;
  float b = 237.7;
  float alpha = ((a * tempC) / (b + tempC)) + log(humidity / 100.0);
  float dewPoint = (b * alpha) / (a - alpha);
  return dewPoint;
  
}

