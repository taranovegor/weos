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

long currentTime;
long loopTime;

//Пины портов
const int back = 11;//red
const int forward = 10;//green
const int up = 8;//yellow
const int down = 9;//blue

int dev_mode = 0;

//Переменные для меню
//Меню заголовки
char* MenuName[16];
//Тип меню, редакт или нет
int MenuType[16];
//Родитель меню
int MenuParent[16];
//Дети меню, в зависисмости от MenuCurPos - зависит и чей он родитель
//Пример MenuChilds[] = {1,3,15,4};
//макс значение пока что 3 (5110 lcd)
int MenuChildMin[16];
int MenuChildMax[16];

//
int curMenu;
//Текущая позиция в меню (Menu Current Position)
int MenuCurPos = 0;
//Статус меню, актив изменение - нет
int MenuStatus = 0;
//Максимальная позиция курсора
int maxPos = 3;

void MenuSetup(){

	MenuName[1]="Menu";
	MenuType[1]=0;
	MenuParent[1]=0;
	MenuChildMin[1]=2;
	MenuChildMax[1]=4;

	MenuName[2]="Часы";
	MenuType[2]=0;
	MenuParent[2]=1;
	MenuChildMin[2]=5;
	MenuChildMax[2]=7;

	MenuName[3]="Экран";
	MenuType[3]=0;
	MenuParent[3]=1;
	MenuChildMin[3]=8;
	MenuChildMax[3]=10;

	MenuName[4]="Прочее";
	MenuType[4]=0;
	MenuParent[4]=1;
	MenuChildMin[4]=0;
	MenuChildMax[4]=0;

	MenuName[5]="Будильник";
	MenuType[5]=0;
	MenuParent[5]=2;
	MenuChildMin[5]=0;
	MenuChildMax[5]=0;

	MenuName[6]="Часы";
	MenuType[6]=0;
	MenuParent[6]=2;
	MenuChildMin[6]=0;
	MenuChildMax[6]=0;

	MenuName[7]="Циферблат";
	MenuType[7]=0;
	MenuParent[7]=2;
	MenuChildMin[7]=0;
	MenuChildMax[7]=0;

	MenuName[8]="Яркость";
	MenuType[8]=0;
	MenuParent[8]=0;
	MenuChildMin[8]=0;
	MenuChildMax[8]=0;

	MenuName[9]="Подсветка";
	MenuType[9]=0;
	MenuParent[9]=0;
	MenuChildMin[9]=0;
	MenuChildMax[9]=0;

	MenuName[10]="Контраст";
	MenuType[10]=0;
	MenuParent[10]=0;
	MenuChildMin[10]=0;
	MenuChildMax[10]=0;

	MenuName[11]="Устройство";
	MenuType[11]=0;
	MenuParent[11]=0;
	MenuChildMin[11]=0;
	MenuChildMax[11]=0;

	MenuName[12]="Перезагрузка";
	MenuType[12]=0;
	MenuParent[12]=0;
	MenuChildMin[12]=0;
	MenuChildMax[12]=0;

}

void printMenu(){
	display.setCursor(2,1);
	display.print(MenuName[curMenu]);
	display.drawFastVLine(0, (8*MenuCurPos), 9, BLACK);
	for(int i = MenuChildMin[curMenu]; i<MenuChildMax[curMenu]+1; i++){
		int b;
		b++;
		display.setCursor(2,8*b);
		display.print(MenuName[i]);
	}
}


void secadd() {
	seconds++;
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

void bAction(){
	if(digitalRead(forward)==LOW&&currentTime>=(loopTime+5)){
		loopTime=currentTime;
		curMenu++;
	}
	if(digitalRead(down)==LOW&&currentTime>=(loopTime+5)){
		loopTime=currentTime;
		MenuCurPos++;
	}
	if(digitalRead(up)==LOW&&currentTime>=(loopTime+5)){
		loopTime=currentTime;
		MenuCurPos--;
	}
}

void setup(){
	//Прерывания по таймеру, добавление секунды
	MsTimer2::set(993, secadd);
	MsTimer2::start();

	//serial!
	Serial.begin(9600);


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

	curMenu=1;

	MenuSetup();
}

void loop(){
	currentTime = millis()/100;
	bAction();
	display.clearDisplay();
	display.print(date(3));
	display.print(":");
	display.print(date(4));
	display.print(":");
	display.print(date(5));
	Serial.println(Serial.read(), DEC);
	//printMenu();
	display.display();
}
