#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <MsTimer2.h>
#include <Encoder.h>

Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 6, 7);

int seconds;
int minutes;
int hours;
int stuck;
int menu_level;

Encoder digitalEncoder(2, 8);

void secadd() {
	seconds++;
}

char* menu_points[] = {"Часы","Экран","Устройство"};

int menu_prev[] = {};

void header(){
	display.drawFastHLine(0,9,LCDWIDTH,BLACK);
	display.fillRect(70, 3, 12, 5, BLACK);
	display.fillRect(71, 4, 3, 3, WHITE);
	display.drawFastVLine(69, 4, 3, BLACK);
	display.setCursor(2,1);
	display.print(menu_points[2]);

	display.setCursor(2,12);
	display.print("ПО тест");
	display.setCursor(2,21);
	display.print("Дисплей N5110");
	display.setCursor(2,30);
	display.print("Обновлено");
	display.setCursor(2,39);
	display.print("17 Окт 21:52");
}

void menu_button(){
	if(digitalRead(11)==LOW&&stuck==1){
		stuck=0;
		if(menu_level==0){
			menu_level=10;
		}
		if(menu_level==10){

		}
	}
}

void menu_print(int mlvl, int ipos, int iline){
	switch(mlvl){
		case 0:
			display.print(hours);
			display.print(":");
			display.print(minutes);
			display.print(":");
			display.print(seconds);
			break;
		case 10:
			switch(iline){
				case 0:
					display.print(menu_points[0]);
					display.print(menu_points[1]);
					display.print(menu_points[2]);
					break;
				case 1:

					break;
			}
			break;
	}
}

void setup(){
	hours=22;
	minutes=35;
	seconds=10;
	//Прерывания по таймеру, добавление секунды
	MsTimer2::set(993, secadd);
	MsTimer2::start();

	//Инциализируем дисплей
	display.begin();
	display.clearDisplay();
	display.setContrast(60);
	display.display();

	//Инциализируем порты
	pinMode(pin_A, INPUT);
	pinMode(pin_B, INPUT);
	pinMode(10, OUTPUT);
	pinMode(11, OUTPUT);
	digitalWrite(10, HIGH);
	digitalWrite(11, HIGH);

	currentTime = millis();
	loopTime = currentTime;

}

void loop(){
	currentTime = millis();
	dEnc = digitalEncoder.read();
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

	if(digitalRead(10)==HIGH&&stuck==0||digitalRead(11)==HIGH&&stuck==0){
		stuck=1;
	}


	if(digitalRead(11)==LOW&&stuck==1){
		stuck=0;
		menu_level=0;
	}

	display.display();
}
