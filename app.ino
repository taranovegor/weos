#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <MsTimer2.h>

Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 6, 7);
//21/34
int seconds;
int minutes;
int hours;
int days;
int months;
int year;

long currentTime;
long loopTime;

//Пины портов
const int back = 11;//red
const int forward = 10;//green
const int up = 8;//yellow
const int down = 9;//blue

int dev_mode = 0;

void secadd() {
	seconds++;
}

const char* menu_points[] = {"Часы","Экран","Устройство"};
int menu_level = 0;
int menu_point = 0;
int ml_array[16];

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

void bAction(){
	if(digitalRead(up)==LOW&&currentTime >= (loopTime + 5)){
		loopTime = currentTime;
		if(menu_point<=0||menu_level==0){
			return;
		}

		menu_point--;
	}
	if(digitalRead(down)==LOW&&currentTime >= (loopTime + 5)){
		loopTime = currentTime;
		if(menu_level==0){
			return;
		}

		menu_point++;
	}
	if(digitalRead(forward)==LOW&&currentTime >= (loopTime + 5)){
		loopTime = currentTime;

		if(menu_level>0){
			ml_array[menu_level] = menu_point;
		}
		menu_level++;
		menu_point=0;
	}
	if(digitalRead(back)==LOW&&currentTime >= (loopTime + 5)){
		loopTime = currentTime;
		if(menu_level<=0){
			return;
		}

		ml_array[menu_level] = 0;
		menu_level--;
		menu_point = ml_array[menu_level];
	}
}

char* printMenu(){
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
	pinMode(back, INPUT);
	pinMode(forward, INPUT);
	pinMode(up, INPUT);
	pinMode(down, INPUT);
	digitalWrite(back, HIGH);
	digitalWrite(forward, HIGH);
	digitalWrite(up, HIGH);
	digitalWrite(down, HIGH);

	//Включение devmode | Зажать кнопки возрата и подтверждения
	if(digitalRead(back)==LOW&&digitalRead(forward)==LOW){
		dev_mode = 1;
	}

	currentTime = millis()/100;
	loopTime = currentTime;

}

void loop(){
	currentTime = millis()/100;
	bAction();
	display.clearDisplay();
	display.print("mp");
	display.print(menu_point);
	display.print("mv");
	display.print(menu_level);
	display.setCursor(0,8);
	display.print(ml_array[0]);
	display.print("-");
	display.print(ml_array[1]);
	display.print("-");
	display.print(ml_array[2]);
	display.print("-");
	display.print(ml_array[3]);
	display.print("-");
	display.print(ml_array[4]);
	display.print("-");
	display.print(ml_array[5]);
	display.print("-");
	display.print(ml_array[6]);
	display.print("-");
	//display.print(ml_array[7]);
	//display.print("-");
	//display.print(ml_array[8]);
	//display.print("-");
	//display.print(ml_array[9]);
	//display.print("-");
	//display.print(ml_array[10]);
	display.print("0");
	display.print("-");
	display.print("1");
	display.print("-");
	display.print("2");
	display.print("-");
	display.print("3");
	display.print("-");
	display.print("4");
	display.print("-");
	display.print("5");
	display.print("-");
	display.print("6");
	display.print("-");
	display.display();
}
