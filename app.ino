#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <MsTimer2.h>

Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 6, 7);


void seconds(){
  display.clearDisplay();
  display.print(millis()/1000);
  display.display();
}

void setup(){
  MsTimer2::set(1000, seconds);
  MsTimer2::start();
  Serial.begin(9600);
  display.begin();
  display.clearDisplay();
  display.setContrast(60);
  display.display();
}

void loop(){
}
