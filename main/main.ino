// OLED lib
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

#define BUTTON_A  0
#define BUTTON_B 16
#define BUTTON_C  2

 
int fsrPin = A0;     // the FSR and 10K pulldown are connected to a0
int fsr0 = 0;
int fsr1 = 0;
int fsr2 = 0;
float fsrMA = 0.0;

int fsrN = 0;
int maxV = 1024;
int awake = 0;
 
void setup(void) {
  Serial.begin(9600); 
  Serial.println("");
  Serial.println("Booting...");  

  // OLED 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
 
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);
 
  // Clear the buffer.
  
 
  Serial.println("IO test");
 
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
 
}

void print(String reading){
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
  
  String message = "";
 
  // We'll have a few threshholds, qualitatively determined
  if (fsr0 < 10) {
    message = " - No pressure";
  } else if (fsr0 < 200) {
    message = " - Light touch";
  } else if (fsr0 < 500) {
    message = " - Light squeeze";
  } else if (fsr0 < 800) {
    message = " - Medium squeeze";
  } else {
    message = " - Big squeeze";
  }

  fsrMA = ((float) fsr0 + (float) fsr1 + (float) fsr2) / 3;
  fsrN = round(fsrMA/ (float) maxV * 100);
  print((String) fsrN);
  
  delay(1000);
  awake = awake + 1;

  if (awake > 5400) {
    display.clearDisplay();
    display.setCursor(7,0);
    display.println("Goodbye");
    display.display();
    delay(3000);
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    ESP.deepSleep(0);
  }
 
} 
