// This example shows a high VGA resolution 3Bit mode
// The VGA3BitI implementation uses the I²S interrupt to transcode a dense frame buffer to the needed
// 8Bit/sample. Using the dense frame buffer allows to fit the big frame buffer in memory at the price of
// a lot cpu performance.
// You need to connect a VGA screen cable to the pins specified below.
// cc by-sa 4.0 license
// bitluni

// including the needed header

#include <ESP32Lib.h>
#include <ArduinoJson.h>
#include <GfxWrapper.h>
#include <Fonts/FreeSerifBold24pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

#include <EEPROM.h>
#include <Wire.h>
// #include "wifi/wifi_network.h"
// #include <update/update.h>

void saveToEEPROM(int address, int value)
{
  EEPROM.writeLong64(address, value);
  EEPROM.commit();
  delay(5); // Wait for the write operation to complete
}

void saveStringToEEPROM(int address, String value)
{
  EEPROM.writeString(address, value);
  EEPROM.commit();
  delay(5); // Wait for the write operation to complete
}

#include <HardwareSerial.h>

bool displayCheck = true;
bool setDisplayTSFRRG = false;
HardwareSerial SerialPort(2); // use UART2

// pin configuration
const int redPins = 14;
const int greenPins = 19;
const int bluePins = 27;
const int hsyncPin = 33;
const int vsyncPin = 32;

// const int redPins[] = {14};
// const int greenPins[] = {19};
// const int bluePins[] = {27};
// const int hsyncPin = 32;
// const int vsyncPin = 33;

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07E0 // 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

const int ledPin = 2;   // Built-in LED on GPIO 2
const int limitPin = 5; // Built-in LED on GPIO 2

const int startPin = 4; // Built-in LED on GPIO 2
const int stopPin = 15; // Built-in LED on GPIO 2

unsigned long previousMillis = 0;
const long interval = 1000; // Interval in milliseconds (20 seconds)
int initCycleCount = 0;
int cycleCount = initCycleCount;
int RealTime = 0;
bool checkRunning = true;

int target = 0;
String style = "";
int cumulate = 0;
int finished = 0;
int rework = 0;
int reworkRate = 0;
int good = 0;
int step = 0;

int initCycleAddress = 0;
int realTimeAddress = 40;
int targetAddress = 80;
int styleAddress = 400;
int cumulateAddress = 140;
int finishedAddress = 180;
int reworkAddress = 220;
int reworkRateAddress = 260;
int goodAddress = 300;
int stepAddress = 340;

VGA3BitI vga;
GfxWrapper<VGA3BitI> gfx(vga, 800, 600);

void setup()
{
  Serial.begin(115200);
  SerialPort.begin(115200);
  EEPROM.begin(512); // Size of EEPROM in bytes

  initCycleCount = EEPROM.readLong64(initCycleAddress);
  RealTime = EEPROM.readLong64(realTimeAddress);
  target = EEPROM.readLong64(targetAddress);
  style = EEPROM.readString(styleAddress);
  cumulate = EEPROM.readLong64(cumulateAddress);
  finished = EEPROM.readLong64(finishedAddress);
  rework = EEPROM.readLong64(reworkAddress);
  reworkRate = EEPROM.readLong64(reworkRateAddress);
  good = EEPROM.readLong64(goodAddress);
  step = EEPROM.readLong64(stepAddress);

  Serial.print("Retrieved value from initCycleCount: ");
  Serial.println(initCycleCount);
  Serial.print("Retrieved value from RealTime: ");
  Serial.println(RealTime);
  Serial.print("Retrieved value from target: ");
  Serial.println(target);
  Serial.print("Retrieved value from style: ");
  Serial.println(style);
  Serial.print("Retrieved value from cumulate: ");
  Serial.println(cumulate);
  Serial.print("Retrieved value from finished: ");
  Serial.println(finished);
  Serial.print("Retrieved value from rework: ");
  Serial.println(rework);
  Serial.print("Retrieved value from reworkRate: ");
  Serial.println(reworkRate);
  Serial.print("Retrieved value from good: ");
  Serial.println(good);
  Serial.print("Retrieved value from step: ");
  Serial.println(step);

  pinMode(ledPin, OUTPUT);
  pinMode(limitPin, INPUT_PULLDOWN);
  pinMode(startPin, INPUT_PULLDOWN);
  pinMode(stopPin, INPUT_PULLDOWN);

  vga.init(vga.MODE800x600, redPins, greenPins, bluePins, hsyncPin, vsyncPin);

  // using adafruit gfx
  gfx.setFont(&FreeSerifBold24pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(0, 50);
  gfx.print("SNOWTEX");

  gfx.setFont(&FreeSans24pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(0, 200);
  gfx.print("Cycle :");

  // rec  Cycle
  gfx.fillRoundRect(240, 160, 150, 50, 0, CYAN);

  // value  Cycle
  gfx.setFont(&FreeSansBold24pt7b);
  gfx.setTextColor(WHITE);
  gfx.setCursor(260, 200);
  gfx.print(initCycleCount);

  // Running
  gfx.setFont(&FreeSans24pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(0, 290);
  gfx.print("Running :");

  // value  Running
  gfx.fillRoundRect(240, 250, 150, 50, 0, CYAN);
  gfx.setFont(&FreeSansBold24pt7b);
  gfx.setTextColor(WHITE);
  gfx.setCursor(260, 290);
  gfx.print(initCycleCount);

  gfx.setFont(&FreeSans24pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(0, 380);
  gfx.print("RealTime :");

  // value  RealTime
  gfx.fillRoundRect(240, 340, 150, 50, 0, CYAN);
  gfx.setFont(&FreeSansBold24pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(260, 380);
  gfx.print(RealTime);

  gfx.setFont(&FreeSans24pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(0, 460);
  gfx.print("Target :");

  // value  Target
  gfx.fillRoundRect(240, 420, 150, 50, 0, CYAN);
  gfx.setFont(&FreeSansBold24pt7b);
  gfx.setTextColor(WHITE);
  gfx.setCursor(260, 460);
  gfx.print(target);

  gfx.setFont(&FreeSans24pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(0, 540);
  gfx.print("Style :");

  // value  Style
  gfx.fillRoundRect(240, 500, 150, 50, 0, CYAN);

  gfx.setFont(&FreeSansBold24pt7b);
  gfx.setTextColor(WHITE);
  gfx.setCursor(240, 540);
  gfx.print(style);

  // last half

  gfx.fillCircle(600, 175, 67, RED);

  gfx.setFont(&FreeMonoBold12pt7b);
  gfx.setTextColor(WHITE);
  gfx.setCursor(537, 180);
  gfx.print("Preparing");

  // Step

  gfx.setFont(&FreeSans18pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(400, 290);
  gfx.print("Step  :");

  gfx.fillRoundRect(640, 260, 100, 40, 0, CYAN);
  // Value
  gfx.setFont(&FreeSans18pt7b);
  gfx.setTextColor(WHITE);
  gfx.setCursor(650, 290);
  gfx.print(step);

  // Cumulate

  gfx.setFont(&FreeSans18pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(400, 340);
  gfx.print("Cumulate  :");

  gfx.fillRoundRect(640, 310, 100, 40, 0, CYAN);
  // Value
  gfx.setFont(&FreeSans18pt7b);
  gfx.setTextColor(WHITE);
  gfx.setCursor(650, 340);
  gfx.print(cumulate);

  // Finished

  gfx.setFont(&FreeSans18pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(400, 390);
  gfx.print("Finished  :");

  gfx.fillRoundRect(640, 360, 100, 40, 0, CYAN);
  // Value
  gfx.setFont(&FreeSans18pt7b);
  gfx.setTextColor(WHITE);
  gfx.setCursor(650, 390);
  gfx.print(finished);

  // Rework

  gfx.setFont(&FreeSans18pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(400, 440);
  gfx.print("Rework  :");

  gfx.fillRoundRect(640, 410, 100, 40, 0, CYAN);
  // Value
  gfx.setFont(&FreeSans18pt7b);
  gfx.setTextColor(WHITE);
  gfx.setCursor(650, 440);
  gfx.print(rework);

  // Rework Rate
  gfx.setFont(&FreeSans18pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(400, 490);
  gfx.print("Rework Rate :");

  gfx.fillRoundRect(640, 460, 100, 40, 0, CYAN);
  // Value
  gfx.setFont(&FreeSans18pt7b);
  gfx.setTextColor(WHITE);
  gfx.setCursor(650, 490);
  gfx.print(reworkRate);

  // Good
  gfx.setFont(&FreeSans18pt7b);
  gfx.setTextColor(YELLOW);
  gfx.setCursor(400, 540);
  gfx.print("Good :");

  // value
  gfx.fillRoundRect(640, 510, 100, 40, 0, CYAN);

  gfx.setFont(&FreeSans18pt7b);
  gfx.setTextColor(WHITE);
  gfx.setCursor(650, 540);
  gfx.print(good);

  // RUN
  gfx.fillRoundRect(395, 555, 60, 40, 5, CYAN);
  gfx.setFont(&FreeMonoBold12pt7b);
  gfx.setTextColor(BLACK);
  gfx.setCursor(400, 580);
  gfx.print("RUN");

  // options
  gfx.fillRoundRect(465, 555, 110, 40, 5, CYAN);
  gfx.setFont(&FreeMonoBold12pt7b);
  gfx.setTextColor(BLACK);
  gfx.setCursor(470, 580);
  gfx.print("Options");

  // System
  gfx.fillRoundRect(590, 555, 100, 40, 5, CYAN);
  gfx.setFont(&FreeMonoBold12pt7b);
  gfx.setTextColor(BLACK);
  gfx.setCursor(600, 580);
  gfx.print("System");

  // Report
  gfx.fillRoundRect(695, 555, 90, 40, 5, CYAN);
  gfx.setFont(&FreeMonoBold12pt7b);
  gfx.setTextColor(BLACK);
  gfx.setCursor(700, 580);
  gfx.print("Report");

  delay(2000);
}
int i = 1;
// the loop is done every frame
int clearWidth = 150;
int clearHeight = 50;

// Get the current cursor position

int cursorX = 240;
int cursorY = 340;
void loop()
{

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    if (cycleCount > -1)
    {
      gfx.fillRoundRect(240, 250, 150, 50, 0, CYAN);
      gfx.setFont(&FreeSansBold24pt7b);
      gfx.setTextColor(WHITE);
      gfx.setCursor(260, 290);
      gfx.println(cycleCount);

      // if (cycleCount < 10)
      // {
      //   gfx.fillRoundRect(400, 250, 150, 50, 0, BLACK);

      //   gfx.setFont(&FreeSansBold24pt7b);
      //   gfx.setTextColor(RED);
      //   gfx.setCursor(400, 300);
      //   gfx.println(cycleCount);
      // }

      if (cycleCount == 0)
      {
        //gfx.fillRoundRect(400, 250, 150, 60, 0, BLACK);
        gfx.fillRoundRect(240, 250, 150, 50, 0, CYAN);
        gfx.setFont(&FreeSansBold24pt7b);
        gfx.setTextColor(WHITE);
        gfx.setCursor(260, 290);
        gfx.println(initCycleCount);
      }
    }
    if (cycleCount == 0)
    {

      digitalWrite(ledPin, HIGH);

      // RealTime++;
      gfx.fillRoundRect(240, 340, 150, 50, 0, CYAN);
      gfx.setFont(&FreeSansBold24pt7b);
      gfx.setTextColor(WHITE);
      gfx.setCursor(260, 380);
      gfx.print(RealTime);
      // Cumelative
      gfx.fillRoundRect(640, 310, 100, 40, 0, CYAN);
      gfx.setFont(&FreeSans18pt7b);
      gfx.setTextColor(WHITE);
      gfx.setCursor(650, 340);
      gfx.print(cumulate);
    }

    Serial.print("Count: ");
    Serial.println(cycleCount);
  }

  if (SerialPort.available() > 0)
  {
    // Read the incoming JSON data
    String jsonInput = SerialPort.readStringUntil('\n');

    // Parse the JSON data
    StaticJsonDocument<512> jsonDocument;
    DeserializationError error = deserializeJson(jsonDocument, jsonInput);

    // Check for parsing errors
    if (error)
    {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
    }
    else
    {
      displayCheck = jsonDocument["setDisplay"];
            setDisplayTSFRRG = jsonDocument["setDisplayTSFRRG"];

      cycleCount = jsonDocument["runningCount"];
      RealTime = jsonDocument["realTime"];
      cumulate = jsonDocument["cumulate"];
      Serial.println(displayCheck);


      if (displayCheck)
      {

        cycleCount = jsonDocument["runningCount"];
        displayCheck = jsonDocument["setDisplay"];
        RealTime = jsonDocument["realTime"];
        cumulate = jsonDocument["cumulate"];
        finished = jsonDocument["finished"];
        rework = jsonDocument["rework"];
        reworkRate = jsonDocument["reworkRate"];
        good = jsonDocument["good"];
        step = jsonDocument["step"];
        style = jsonDocument["style"].as<String>();
        target = jsonDocument["target"];
        initCycleCount = jsonDocument["cycleCount"];

        // Serial.print("Retrieved value from initCycleCount: ");
        // Serial.println(initCycleCount);
        // Serial.print("Retrieved value from RealTime: ");
        // Serial.println(RealTime);
        // Serial.print("Retrieved value from target: ");
        // Serial.println(target);
        // Serial.print("Retrieved value from style: ");
        // Serial.println(style);
        // Serial.print("Retrieved value from cumulate: ");
        // Serial.println(cumulate);
        // Serial.print("Retrieved value from finished: ");
        // Serial.println(finished);
        // Serial.print("Retrieved value from rework: ");
        // Serial.println(rework);
        // Serial.print("Retrieved value from reworkRate: ");
        // Serial.println(reworkRate);
        // Serial.print("Retrieved value from good: ");
        // Serial.println(good);
        // Serial.print("Retrieved value from step: ");
        // Serial.println(step);

        // Serial.print("Retrieved value from step: ");
        // Serial.println(style);

        saveToEEPROM(targetAddress, target);
        target = EEPROM.readLong64(targetAddress);
        delay(100);

        saveStringToEEPROM(styleAddress, style);
        style = EEPROM.readString(styleAddress);
        delay(100);

        saveToEEPROM(cumulateAddress, cumulate);
        cumulate = EEPROM.readLong64(cumulateAddress);
        delay(100);

        saveToEEPROM(finishedAddress, finished);
        finished = EEPROM.readLong64(finishedAddress);
        delay(100);

        saveToEEPROM(reworkAddress, rework);
        rework = EEPROM.readLong64(reworkAddress);
        delay(100);

        saveToEEPROM(reworkRateAddress, reworkRate);
        reworkRate = EEPROM.readLong64(reworkRateAddress);
        delay(100);

        saveToEEPROM(goodAddress, good);
        good = EEPROM.readLong64(goodAddress);
        delay(100);

        saveToEEPROM(stepAddress, step);
        step = EEPROM.readLong64(step);
        delay(100);

        saveToEEPROM(initCycleAddress, initCycleCount);
        initCycleCount = EEPROM.readLong64(initCycleAddress);
        delay(100);

        saveToEEPROM(realTimeAddress, RealTime);
        RealTime = EEPROM.readLong64(realTimeAddress);
        delay(100);

        ESP.restart();
      }

      if (setDisplayTSFRRG)
      {

        finished = jsonDocument["finished"];
        rework = jsonDocument["rework"];
        reworkRate = jsonDocument["reworkRate"];
        good = jsonDocument["good"];
        style = jsonDocument["style"].as<String>();
        saveToEEPROM(targetAddress, target);

        target = EEPROM.readLong64(targetAddress);
        delay(100);

        saveStringToEEPROM(styleAddress, style);
        style = EEPROM.readString(styleAddress);
        delay(100);


        saveToEEPROM(finishedAddress, finished);
        finished = EEPROM.readLong64(finishedAddress);
        delay(100);

        saveToEEPROM(reworkAddress, rework);
        rework = EEPROM.readLong64(reworkAddress);
        delay(100);

        saveToEEPROM(reworkRateAddress, reworkRate);
        reworkRate = EEPROM.readLong64(reworkRateAddress);
        delay(100);

        ESP.restart();
      }

      // if (displayCheck)
      // {
      //   gfx.fillRoundRect(240, 160, 150, 50, 0, CYAN);
      //   gfx.setFont(&FreeSansBold24pt7b);
      //   gfx.setTextColor(WHITE);
      //   gfx.setCursor(260, 200);
      //   gfx.print(initCycleCount);
      //   digitalWrite(ledPin, LOW);
      //   displayCheck = false;
      // }
    }
  }
}
