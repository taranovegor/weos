#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <MsTimer2.h>

Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 6, 7);

int seconds;
int minutes;
int hours;
int days;
int months;
int year;

const int pin_A = 2;
const int pin_B = 8;
unsigned char encoder_A;
unsigned char encoder_B;
unsigned char encoder_A_prev=0;

int stuck;
int menu_level;

int currentTime;
int loopTime;
int test;
int posEnc = 0;
int nposEnc;

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
	display.print("24 Окт 21:52");//17
}

int date(int arg){
	//Если секунд больше 59, аннулируем переменную seconds и добавим минуту
	if(seconds>59){
		seconds=0;
		minutes++;
	}
	//Если минут > 59, аннулируем переменную minutes и добавим час
	if(minutes>59){
		minutes=0;
		hours++;
	}
	//Если часов > 23, тогда аннулируем переменную hours и добавим день
	if(hours>23){
		hours=0;
		days++;
	}
	//Добавляем массив с количеством дней в месяце, ноль для того что бы небыло мороки с отсётом месяцев с нуля (0-11 вместо 1-12)
	const int daysinMonths[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	//Если день > количества дней в месяц [текущий], обнуляем days и добавляем месяц
	//Вводит в ступор инциализация месяца, разобрать нужно
	if(days>daysinMonths[months]){
		days=1;
		months++;
	}
	//Если месяц > 12, аннулируем months до 1 и добавляем год
	if(months>12){
		months=1;
		year++;
	}
	//Готовим (на печи) строку для возращения
	//year = 0 months = 1 days = 2 hours = 3 minutes = 4 seconds = 5
	int retstr[] = {year, months, days, hours, minutes, seconds};
	//Возращаем строку
	return retstr[arg];
}

void setup(){
	seconds = 10;
	minutes = 10;
	hours = 22;
	days = 23;
	months = 10;
	year = 2015;

	//Прерывания по таймеру, добавление секунды
	MsTimer2::set(993, secadd);
	MsTimer2::start();

	//Инциализируем дисплей
	display.begin();
	display.clearDisplay();
	display.setContrast(60);
	display.display();
pinMode(pin_A, INPUT);
  pinMode(pin_B, INPUT);
	//Инциализируем порты
	pinMode(10, OUTPUT);
	pinMode(11, OUTPUT);
	digitalWrite(10, HIGH);
	digitalWrite(11, HIGH);

	//
	currentTime = millis()/10;
	loopTime = currentTime;
}

void loop(){
	display.clearDisplay();
	currentTime = millis()/10;

	if(digitalRead(10)==HIGH&&stuck==0||digitalRead(11)==HIGH&&stuck==0){
		stuck=1;
	}


	if(digitalRead(11)==LOW&&stuck==1){
		stuck=0;
		menu_level=0;
	}
	if(currentTime >= (loopTime + 8)){
		encoder_A = digitalRead(pin_A);
		encoder_B = digitalRead(pin_B);
		if((!encoder_A) && (encoder_A_prev)){
			if(encoder_B){
				test++;
			}
			if(encoder_A){
				test=0;
			}
		}
		display.print("f");
		encoder_A_prev = encoder_A;
		loopTime = currentTime;
	}

	display.print(loopTime);
	display.setCursor(2,10);
	display.print(currentTime);
	display.setCursor(2,20);
	display.print(test);
	display.setCursor(2,30);
	display.print(encoder_A);
	display.print(encoder_B);
	display.display();
}
