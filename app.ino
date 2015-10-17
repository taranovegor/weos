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

/*void header() {
	display.drawRect(0, 0, LCDWIDTH, 11, BLACK);
	display.fillRect(69, 3, 12, 5, BLACK);
	display.fillRect(70, 4, 3, 3, WHITE);
	display.drawFastVLine(68, 4, 3, BLACK);
	display.setCursor(2,2);
	display.print("Устройство");
	//display.drawFastVLine(0,12,11,BLACK);
	//12
	display.setCursor(2,14);
	display.print("ПО:weOS 0.1");
	//+11=23
	display.setCursor(2,25);
	display.print("Дисплей:N5110");
	//+11=34
	display.setCursor(2,36);
	display.print("Обн:17 Окт");

}*/

char* menu_points[] = {"Часы","Экран","Устройство"};

void header(){
	display.drawFastHLine(0,9,LCDWIDTH,BLACK);
	display.fillRect(69, 3, 12, 5, BLACK);
	display.fillRect(70, 4, 3, 3, WHITE);
	display.drawFastVLine(68, 4, 3, BLACK);
	display.setCursor(2,1);
	display.print(menu_points[2]);
}



void setup(){

	//Прерывания по таймеру, добавление секунды
	MsTimer2::set(993, secadd);
	MsTimer2::start();

	//Инциализируем дисплей
	display.begin();
	display.clearDisplay();
	display.setContrast(60);
	display.display();

	//Инциализируем порты
	pinMode(10, OUTPUT);
	pinMode(11, OUTPUT);
	digitalWrite(10, HIGH);
	digitalWrite(11, HIGH);

}

void loop(){
	int dEnc;//digital Encoder
	int posEnc = 0;//position Encoder
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
	if(digitalRead(10)==LOW&&stuck==1){
		stuck=0;
		menu_level=1;
	}

	if(digitalRead(11)==LOW&&stuck==1){
		stuck=0;
		menu_level=0;
	}

	if(dEnc != posEnc){
		header();

		posEnc = dEnc;
	}


	header();
	display.display();
/*
	if(menu_level==0){
		display.print(hours);
		display.print(":");
		display.print(minutes);
		display.print(":");
		display.print(seconds);
		display.display();
	}
	if(menu_level==1){
		header();
		//display.drawRect(0, 12, LCDWIDTH, 11, BLACK);
		display.setCursor(2,14);
		display.print(menu_points[2]);
		display.display();
	}
	*/
}
