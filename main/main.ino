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
int fsr0 = 0;
int fsr1 = 0;
int fsr2 = 0;
float fsrMA = 0.0;

// max voltage (based on pulldown resistor) seen on 
// analog (A0) port, used to normlaize display
int maxV = 1024;  
int fsrN = 0;

int awake = 0;
// shutdown after this long
int shutdown_seconds = 5400; 
 
void setup(void) {
  Serial.begin(9600); 
  Serial.println("");
  Serial.println("Booting...");  

  // OLED 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);
 
  // set pbuttons for future use
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
 
}

void print(String reading){
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
 
void loop(void) {
  fsr2 = fsr1;
  fsr1 = fsr0;
  fsr0 = analogRead(fsrPin);  
 
  Serial.print("Analog reading = ");
  Serial.println(fsr0);  

  // calculate 3 point moving average, and normalize off max voltage
  fsrMA = ((float) fsr0 + (float) fsr1 + (float) fsr2) / 3;
  fsrN = round(fsrMA/ (float) maxV * 100);
  print((String) fsrN);
  
  delay(1000);
  awake = awake + 1;

  if (awake > shutdown_seconds) {
    display.clearDisplay();
    display.setCursor(7,0);
    display.println("Goodbye...");
    display.display();
    delay(3000);
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    ESP.deepSleep(0);
  }
 
} 
