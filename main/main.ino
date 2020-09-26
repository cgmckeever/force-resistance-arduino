// config
#include "config.h"
#include <string>

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

void print(const char *line1, const char *line2);

void setup(void) {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Booting...");

  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  activate();
  delay(2000);

  // set buttons
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

}

void loop(void) {
  if(!digitalRead(BUTTON_A)) calibrate();
  if(!digitalRead(BUTTON_B)) displayToggle();
  if(!digitalRead(BUTTON_C)) reduceReadingInterval();

  if (displayOn) {
    getResistance();
    String s = String(fsrNormalized);
    print("Resistance", s.c_str());

    if (fsrNoChange > sleepPolls) displayToggle();
  } else if (fsrNormalized > 80) displayToggle();
}

void getResistance() {
  int fsrTotal = 0;

  for(int i = 1; i <= fsrReadings; i++) {
    fsrTotal += analogRead(fsrPin);
    delay(150);
  }

  fsrLevel = floor(fsrTotal / fsrReadings);

  fsrNormalized = floor(100 * fsrLevel / maxV);
  if (fsrNormalized > 100) fsrNormalized = 100;

  Serial.print("Analog reading = ");
  Serial.println(fsrLevel);

  if (fsrNormalized == fsrLast) {
    fsrNoChange += 1;
  }else {
    fsrNoChange = 0;
  }

  fsrLast = fsrNormalized;
}

void calibrate(){
  printCalibration();
  print("Hold Break", "5 seconds");
  delay(3000);

  maxV = analogRead(fsrPin) + 20;
  printCalibration();
}

void reduceReadingInterval() {
  if (fsrReadings > 1) {
   fsrReadings -= 1;
  } else {
    fsrReadings = fsrMaxReadings;
  }

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
  display.ssd1306_command(SSD1306_DISPLAYON);
  displayOn = true;
  Serial.println("Screen on");

  fsrNoChange = 0;

  print("DIYPELOTON");
  delay(2000);
  printCalibration();
  printInterval();
}

void displayToggle() {
  if (displayOn) {
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