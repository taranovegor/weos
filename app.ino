#include <SPI.h>
#include <TFT_ILI9163C.h>
#include <MsTimer2.h>
//#include <MemoryFree.h>

// Color's
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF

// Pin's
//buttons
#define down 0
#define up 1
#define ok 2
#define back 4
//other
#define vibration 3
#define backlight 9
/*
---|LCD pin's
__CS - 2	__SDA 11
__DC - 3	__SCK 13
__RST - 4
---|Button's pin
0 - down	2 - ok
1 - up		4- back
*/
#define __CS  10
#define __DC  6
#define __RST 8

TFT_ILI9163C lcd = TFT_ILI9163C(__CS, __DC, __RST);

//
int seconds;
int minutes;
int hours;
int day;
int month;
int year;

int minuteFixed = 999;

int devMode = 0;

void MenuSetup(){
	/*
	MenuNames[]=;
	MenuTypeCode[]=;
	MenuValue[]=;
	MenuParent[]=;
	MenuChildFirst[]=;
	MenuChildEnd[]=;
	*/
}

void timerSeconds() {
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
		day++;
	}
	//Добавляем массив с количеством дней в месяце, ноль для того что бы небыло мороки с отсётом месяцев с нуля (0-11 вместо 1-12)
	const int daysinMonths[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	//Если день > количества дней в месяц [текущий], обнуляем days и добавляем месяц
	//Вводит в ступор инциализация месяца, разобрать нужно
	if(day>daysinMonths[month]){
		day=1;
		month++;
	}
	//Если месяц > 12, аннулируем months до 1 и добавляем год
	if(month>12){
		month=1;
		year++;
	}
	//Готовим (на печи) строку для возращения
	//year = 0 months = 1 days = 2 hours = 3 minutes = 4 seconds = 5 
	int retstr[] = {year, month, day, hours, minutes, seconds};
	//Возращаем строку
	return retstr[arg];
}

void setup(){
	//initialization timer for clock
	MsTimer2::set(993, timerSeconds);
	MsTimer2::start();

	lcd.begin();
	analogWrite(backlight, 50);
	//Pin's MODE initialization
	pinMode(down, OUTPUT);
	pinMode(up, OUTPUT);
	pinMode(ok, OUTPUT);
	pinMode(back, OUTPUT);
	//step2
	digitalWrite(down, HIGH);
	digitalWrite(up, HIGH);
	digitalWrite(ok, HIGH);
	digitalWrite(back, HIGH);

	if(digitalRead(back)==LOW&&digitalRead(ok)==LOW){
		devMode = 1;
		seconds=50;
	}
}

void loop(){
	if(date(5)!=minuteFixed){
		lcd.setTextSize(4);

		lcd.setTextColor(BLACK);
		lcd.setCursor(0,0);
		if(date(5)<10&&date(5)!=0){
			lcd.setCursor(24,0);
			lcd.print(minuteFixed);
		}
		else if(date(5)==10){
			lcd.print("09");
		}
		else{
			lcd.print(minuteFixed);
		}

		minuteFixed=date(5);

		lcd.setCursor(0,0);
		lcd.setTextColor(WHITE);
		if(date(5)<10){
			lcd.print("0");
		}
		lcd.print(date(5));

		/*lcd.setTextSize(4);
		lcd.setCursor(0,0);
		if(date(5)<10){
			lcd.print("0");
			lcd.setCursor(24,0);
		}
		if(date(5)==10){
			lcd.setTextColor(BLACK);
			lcd.print("0");
			lcd.setCursor(0,0);
		}

		lcd.setTextColor(BLACK);
		lcd.print(minuteFixed);
		minuteFixed=date(5);
		if(date(5)<10){
			lcd.setCursor(24,0);
		}
		else{
			lcd.setCursor(0,0);
		}
		lcd.setTextColor(WHITE);
		lcd.print(date(5));*/
	}
}