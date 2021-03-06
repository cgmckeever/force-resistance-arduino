// config
#include "config.h"

// OLED lib
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

bool displayOn = false;

int fsrLevel = 0;
int fsrNormalized = 0;
int fsrLast = 0;
int fsrReadings = fsrMaxReadings;
int fsrNoChange = 0;

int sleepPolls = 0;

void print(const char *line1, const char *line2);

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Booting...");

  activate();

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
}

void loop() {
  if(!digitalRead(BUTTON_A)) calibrate();
  if(!digitalRead(BUTTON_B)) displayToggle();
  if(!digitalRead(BUTTON_C)) reduceReadingInterval();

  getResistance();
  if (displayOn) {
    if (fsrNoChange > sleepPolls) displayToggle();
  } else if (fsrNormalized > 80) displayToggle();
}

void getResistance() {
  int fsrTotal = 0;

  for(int i = 1; i <= fsrReadings; i++) {
    fsrTotal += analogRead(fsrPin);
    delay(pollingInterval);
  }

  fsrLevel = floor(fsrTotal / fsrReadings);
  Serial.print("Analog reading = ");
  Serial.println(fsrLevel);

  fsrNormalized = floor(100 * fsrLevel / maxV);
  if (fsrNormalized > 100) fsrNormalized = 100;

  String s = String(fsrNormalized);
  print("Resistance", s.c_str());

  int increment = fsrNormalized == fsrLast ? 1 : 0;
  fsrNoChange += increment;
  fsrLast = fsrNormalized;
}

void setSleepPolls() {
  int pollMS = (pollingInterval * fsrReadings);
  sleepPolls = 1000 * secondsTillSleep / pollMS;
  fsrNoChange = 0;

  Serial.print("Sleep Polls: ");
  Serial.println(sleepPolls);
}

void calibrate() {
  int minV = analogRead(fsrPin);
  printCalibration();
  print("Hold Break", "5 seconds");
  delay(3000);

  maxV = analogRead(fsrPin) - minV;
  printCalibration();
}

void reduceReadingInterval() {
  if (fsrReadings > 1) {
   fsrReadings -= 1;
  } else {
    fsrReadings = fsrMaxReadings;
  }

  setSleepPolls();
  printInterval();
}

int centerText(const char *buf) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
  return floor((130 - w)/2);
}

void printAt(String text, int X, int Y) {
  display.setCursor(X, Y);
  display.println(text);
  display.display();
}

void print(const char *line1, const char *line2 = "") {
  if (displayOn) {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);

    int x = centerText(line1);
    printAt(line1, x, 0);

    x = centerText(line2);
    printAt(line2, x, 18);
  }
}

void printCalibration() {
  String s = "max: " + String(maxV);
  print("Calibrate", s.c_str());
  delay(2000);
}

void printInterval() {
  String s = "-- " + String(fsrReadings) + " --";
  print("Polling", s.c_str());
  delay(2000);
}

void activate() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.ssd1306_command(SSD1306_DISPLAYON);
  displayOn = true;
  Serial.println("Screen on");

  print("DIYPELOTON");
  delay(2000);
  printCalibration();
  setSleepPolls();
  printInterval();
}

void displayToggle() {
  if (displayOn) {
    Serial.println("Sleep Activated");
    print("Goodbye...");
    delay(2000);
    display.clearDisplay();
    display.display();
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    displayOn = false;
  } else {
    activate();
  }
}