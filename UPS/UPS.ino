#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "UPS.h"


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned int voltageADC = 0;
float voltage = 0.00;
unsigned char voltagePercentage = 0;
String readString = "";
String IP = "000.000.000.000";
char temperature = 0;
char source = 0;

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    //    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.drawBitmap( 0, 0, tm_bmp, TM_WIDTH, TM_HEIGHT, 1);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(78, 0);
  display.println("NANO UPS");
  display.display();
  delay(5000);


  pinMode(SourcePin, INPUT);
}

void loop() {
  while (Serial.available()) {
    delay(3);
    if (Serial.available() > 0) {
      char c = Serial.read();
      readString += c;
    }
  }

  if (readString != "") {
    //      Serial.println(readString);

    //    {"sensor":"gps","time":1351824120,"data":[48.756080,2.302038]}

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, readString);

    // Test if parsing succeeds.
    if (error) {
      //        Serial.print(F("deserializeJson() failed: "));
      //        Serial.println(error.f_str());
      return;
    }

    //      const char* sensor = doc["sensor"];
    //      long time = doc["time"];
    //      double latitude = doc["data"][0];
    //      double longitude = doc["data"][1];

    temperature = doc["temperature"];

  }
  readString = "";

  getVolts();
  draw();
  sendData();
  delay(2000);
}

void sendData() {
  StaticJsonDocument<200> doc;
  doc["voltage"] = voltage;
  doc["voltagePercentage"] = voltagePercentage;
  doc["voltageADC"] = voltageADC;
  doc["source"] = source;
  serializeJson(doc, Serial);
  Serial.println();
}

void draw() {
  display.clearDisplay();
  display.drawBitmap( 96, 0, router_bmp, ROUTER_WIDTH, ROUTER_HEIGHT, 1);
  display.drawBitmap( 0, 0, battery_bmp, BATTERY_WIDTH, BATTERY_HEIGHT, 1);
  drawVoltage();
  drawPercentage();
  drawPower();
  drawServer();
  drawLevel();
  display.display();      // Show initial text
  delay(100);
}

void drawLevel() {
  char level = map (voltagePercentage, 0, 100, 0, 23);
  display.fillRoundRect(3, 23 - level + 6, 10, level, 2, SSD1306_INVERSE);
}

void drawServer() {
  char buffer[5];
  sprintf(buffer, "%02d C", temperature);
  display.setTextSize(1);
  display.setCursor(96, 0);
  display.println(buffer);
  display.drawCircle(111, 1, 1, SSD1306_WHITE);
}

void drawPower() {
  if (source) {
    display.drawBitmap(72 , 16, plug_bmp, PLUG_WIDTH, PLUG_HEIGHT, 1);
  } else {
    for (char i = 0; i < 72; i += 6) {
      display.drawLine( 20 + i, 16, 23 + i, 16, SSD1306_WHITE);
    }
  }
}

void drawPercentage() {
  char buffer[5];
  sprintf(buffer, "%d%%", voltagePercentage);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 0);
  display.println(buffer);
}

void drawVoltage() {
  char buffer[6];
  sprintf(buffer, "%d.%02dV", (int)voltage, (int)(voltage * 100) % 100);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 24);
  display.println(buffer);
}

float getVolts() {
  source = digitalRead(SourcePin);
  voltageADC = analogRead(ADCPin);
  voltage = (float)voltageADC * 12.6 / FULL_VALUE;
  voltagePercentage = ADC2PERCENTAGE(voltageADC);
  //  Serial.print("ADC: ");
  //  Serial.println(voltageADC);
  //  Serial.print("Voltage: ");
  //  Serial.println(voltage);
  //  Serial.print("Percentage: ");
  //  Serial.println(voltagePercentage);
}
