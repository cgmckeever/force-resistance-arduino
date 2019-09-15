// config
#include "config.h"

// OLED lib
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

 
void setup(void) {
  Serial.begin(9600); 
  Serial.println("");
  Serial.println("Booting...");  

  // OLED 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  displayOn();
  delay(2000);
 
  // set buttons
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  if (defaultMode != 'resistance') fsrCount = 0; 
 
}

void loop(void) {

  if(!digitalRead(BUTTON_A)) calibrate();
  if(!digitalRead(BUTTON_B)) displayOff();
  if(!digitalRead(BUTTON_C)) reduceInterval();

  getResistance();

  if (fsrCount > 0){
    printResistance(fsrN);
  }else {
    printLevel(fsrLevel);
  }
  
  delay(1000);

  if ((displayState == 0) && (fsrN > 80)) {
    fsrCount = fsrCountMax;
    fsrHeld = 0;
    displayOn();
  }else if (fsrHeld > fsrTimeout) {
    if (displayState == 1) displayOff();
    fsrHeld = 1;
  }
  
} 


//////////////
//

void getResistance(void){
  fsrTotal = 0;
  for(int i = 1; i < fsrCount; i++) {
    fsr[i-1] = fsr[i];
    fsrTotal = fsrTotal + fsr[i];
  }

  int last = lastIndex();
  
  fsr[last] = analogRead(fsrPin);
  Serial.print("Analog reading = ");
  Serial.println(fsr[last]); 
  
  fsrTotal = fsrTotal + fsr[last]; 
 
  // calculate fsrCount moving average, and normalize off max voltage
  fsrMA = (float) fsrTotal / (last + 1);
  Serial.println((String) (last + 1) + " Moving Average: " + (String) fsrMA);
  
  fsrLast = fsrN;
  fsrN = round(fsrMA / (float) maxV * 100);
  if (fsrN > 100) fsrN = 100;
  Serial.println("Normalized: " + (String) fsrN);
 
  if (fsrN == fsrLast) {
    fsrHeld = fsrHeld + 1;
  }else{
    fsrHeld = 0;
  }

  //calculate fsr step range
  fsrLevel = round((float) fsrMA / levelV);
  Serial.println("Level: " + (String) fsrLevel);
}

String pad(String string, int len){
  int padding = len - string.length();
  for(int i = 0; i < len; i++) {
    string = ' ' + string;  
  }
  return string;
}

void print(int line1X, String line1, int line2X, String line2){
  if (displayState == 1) {
    display.clearDisplay();  
    display.setTextSize(2);
    display.setTextColor(WHITE);
    
    display.setCursor(line1X,0);
    display.println(line1);
  
    display.setCursor(line2X,18);
    display.println(line2);
    display.display();
  }
}

void printResistance(int reading){
  String resistance = (String) reading;
  resistance = pad(resistance, 2); 
  print(7, "Resistance", 40, resistance);
}

void printLevel(int reading){
  String level = (String) reading;
  level = pad(level, 2);
  print(30, "Level", 45, level);
}

void calibrate(){
  printCalibration();  
  print(5, "Hold Break", 3, "5 seconds");
  delay(3000);
  
  maxV = analogRead(fsrPin) + 20;
  printCalibration();
}

void printCalibration(){
  print(7, "Calibrate", 0, "max: " + (String) maxV);
  delay(2000);
}

void reduceInterval(){
  if (fsrCount > 0) {
   fsrCount = fsrCount - 1;
  }else {
    fsrCount = fsrCountMax;
  }
  printInterval();    
}

int lastIndex(void){
  int lastIndex = fsrCount - 1;
  if (lastIndex < 0){
    lastIndex = 0;
  }
  return lastIndex;
}

void printInterval(void){
  if (fsrCount > 0){
    print(15, "Interval", 30, "--" + (String) fsrCount + "--");
  }else{
    print(0, "Levels " + (String) levels, 0, "");
  }
  
  delay(2000);
}

void displayOff(void){
  fsrCount = 1;
  print(5, "Goodbye...", 37, "");
  delay(2000);
  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  displayState = 0;
}

void displayOn(void){
  display.ssd1306_command(SSD1306_DISPLAYON);
  displayState = 1;
  Serial.println("Screen on");

  print(7, "DIYPELOTON", 37, "");
  delay(2000);
  printInterval();
  printCalibration();
}
