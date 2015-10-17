#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <MsTimer2.h>

Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 6, 7);

int seconds;
int minutes;
int hours;
int stuck;
int menu_level;

void secadd() {
	seconds++;
}

void stuck_off(){
	stuck=1;
}

void setup(){
	seconds=0;
	stuck=0;
	menu_level=0;
	MsTimer2::set(993, secadd); // 500ms period
	MsTimer2::set(500, stuck_off);
	MsTimer2::start();

	Serial.begin(9600);
	display.begin();
	display.clearDisplay();
	display.setContrast(60);
	display.display();
	pinMode(10, OUTPUT);
	pinMode(11, OUTPUT);
	digitalWrite(10, HIGH);
	digitalWrite(11, HIGH);
	if(digitalRead(10)==LOW){
		display.print("hi!");
		display.display();
	}
}

void loop(){
	display.clearDisplay();

	if(seconds>59){
		seconds=0;
		minutes++;
	}

	if(seconds>59){
		minutes++;
		seconds=0;
	}

	if(minutes>59){
		minutes=0;
		hours++;
	}

	if(hours>23){
		hours=0;
	}

	if(digitalRead(10)==LOW&&stuck==1){
		stuck=0;
		menu_level=1;
	}

	if(digitalRead(11)==LOW&&stuck==1){
		stuck=0;
		menu_level=0;
	}

	if(menu_level==0){
		display.print(hours);
		display.print(":");
		display.print(minutes);
		display.print(":");
		display.print(seconds);
		display.display();
	}
	if(menu_level==1){
		display.print("test");
		display.print(stuck);
		display.display();
	}
}
