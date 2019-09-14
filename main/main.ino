// OLED lib
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

#define BUTTON_A  0
#define BUTTON_B 16
#define BUTTON_C  2

// the FSR and pulldown resistor are connected to a0 
int fsrPin = A0;    

// moving average variables
#define fsrCountMax 5
float fsrMA = 0.0;
int fsrTotal = 0;
int fsrCount = fsrCountMax;
int fsr[fsrCountMax]; 

// max voltage (based on pulldown resistor) seen on 
// analog (A0) port, used to normlaize display
int maxV = 1024;  
int fsrN = 0;
int fsrLast = 0;

// timeout 
#define fsrTimeout 10 * 60
int fsrHeld = 0;
int displayState = 1;

 
void setup(void) {
  Serial.begin(9600); 
  Serial.println("");
  Serial.println("Booting...");  

  // OLED 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(1000);
  displayOn();
 
  // set pbuttons for future use
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
 
}

void print(String reading){
  if (displayState == 1) {
    // pad reading for display formating
    int len = 3 - reading.length();
    for(int i = 0; i < len; i++) {
      reading = ' ' + reading;  
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
  
    display.setCursor(7,0);
    display.println("RESISTANCE");
  
    display.setCursor(37,18);
    display.println(reading);
    display.display();
  }
}

void calibrate(int init){
  display.clearDisplay();
  display.setCursor(7,0);
  display.println("CALIBRATE");

  display.setCursor(0,18);
  display.println("max: " + (String) maxV + "v");
  display.display();

  
  if (init){
    delay(2000);
    display.setCursor(0,18);
    display.println("                    ");
    display.println("Press Break Fully");
    display.display();
  }
  
}

void displayOff(void){
  display.clearDisplay();
  display.setCursor(7,0);
  display.println("Goodbye...");
  display.display();
  delay(3000);
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  displayState = 0;
}

void displayOn(void){
  display.begin();
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("DIYPELOTON");
  display.display();
  delay(3000);
  print("--" + (String) fsrCount + "--");
  displayState = 1;
  delay(1000);
}
 
void loop(void) {

  fsrTotal = 0;
  for(int i = 1; i < fsrCount; i++) {
    fsr[i-1] = fsr[i];
    fsrTotal = fsrTotal + fsr[i];
  }
  
  fsr[fsrCount-1] = analogRead(fsrPin);
  Serial.print("Analog reading = ");
  Serial.println(fsr[fsrCount-1]); 
  
  fsrTotal = fsrTotal + fsr[fsrCount-1]; 
 
  // calculate fsrCount moving average, and normalize off max voltage
  fsrMA = (float) fsrTotal / fsrCount;
  Serial.println((String) fsrCount + " Moving Average: " + (String) fsrMA);
  fsrLast = fsrN;
  fsrN = round(fsrMA / (float) maxV * 100);
  print((String) fsrN);
  Serial.println("Resistance: " + (String) fsrN);

  if (fsrN == fsrLast) {
    fsrHeld = fsrHeld + 1;
  }else{
    fsrHeld = 0;
  }
  
 
  if(!digitalRead(BUTTON_A)){
    calibrate(1);
    delay(2000);
    maxV = analogRead(fsrPin);
    calibrate(0);
    delay(2000);
  }

  if(!digitalRead(BUTTON_B)){
    displayOff();
  }

  if(!digitalRead(BUTTON_C)){
    if (fsrCount > 1) {
     fsrCount = fsrCount - 1;
     print("--" + (String) fsrCount + "--"); 
    }
  }

  delay(1000);

  if (fsrHeld > fsrTimeout) {
    if (displayState == 1) {
      fsrCount = 1;
      displayOff();
    }
    fsrHeld = 1;
  }else if ((displayState == 0) && (fsrN > 90)) {
    fsrCount = fsrCountMax;
    displayOn();
  }
 
} 
