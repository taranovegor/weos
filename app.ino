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
unsigned int seconds;
unsigned int minutes;
unsigned int hours;
unsigned int day;
unsigned int numWeekDay;
unsigned int month;
unsigned int year;

unsigned int minuteFixed = 0;
unsigned int hourFixed = 0;
unsigned int dayFixed = 0;

boolean printDates = false;

int devMode = 0;

//Инциализация меню

char* MenuName[3];

int MenuType[3];

int MenuParent[3];

int MenuChildFirst[3];

int MenuChildLast[3];

void MenuSetup(){
	MenuNames[0]="";
	MenuType[0]=0;
	MenuParent[0]=0;
	MenuChildFirst[0]=0;
	MenuChildLast[0]=0;
}

void timerSeconds() {
	seconds++;
}

static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9') v = *p - '0';
  return 10 * v + *++p - '0';
}

int menu_level = 0;

unsigned int date(int arg){
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
		numWeekDay++;
	}
	//Добавляем массив с количеством дней в месяце, ноль для того что бы небыло мороки с отсётом месяцев с нуля (0-11 вместо 1-12)
	const int daysinMonths[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	//Если день > количества дней в месяц [текущий], обнуляем days и добавляем месяц
	//Вводит в ступор инциализация месяца, разобрать нужно
	if(day>daysinMonths[month]||day==0){
		day=1;
		month++;
	}
	if(numWeekDay>7||numWeekDay==0){
		numWeekDay=1;
	}
	//Если месяц > 12, аннулируем months до 1 и добавляем год
	if(month>12||month==0){
		month=1;
		year++;
	}
	//Готовим (на печи) строку для возращения
	//year = 0 months = 1 days = 2 numWeekDay = 3 hours = 4 minutes = 5 seconds = 6
	int retstr[] = {year, month, day, numWeekDay, hours, minutes, seconds};
	//Возращаем строку
	return retstr[arg];
}

const char* namesDays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char* namesMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void (){
	//lcd.drawFastHLine(24,95,128, RED);
	//lcd.drawFastHLine(24,33,128, RED);
	if(date(2)!=dayFixed||printDates==false){
		lcd.clearScreen();
		//width symbols (3)== 34px
		lcd.setTextSize(2);
		lcd.setCursor(70,34);//6232
		lcd.print(namesDays[date(3)-1]);
		lcd.setCursor(70,50);
		lcd.print(date(2));
		lcd.setCursor(70,66);
		lcd.print(namesMonths[date(1)-1]);
		//Обновляем переменные
		dayFixed=date(2);//
		printDates=false;//Костыль заканчивается в минутах
	}
	if(date(4)!=hourFixed||printDates==false){
		lcd.setTextSize(4);
		lcd.setTextColor(BLACK);

		lcd.setCursor(24,34);//x,32-true-pos
		if(date(4)<10&&date(4)!=0){
			lcd.setCursor(48,34);
			if(hourFixed!=99){
				lcd.print(hourFixed);
			}
		}
		else if(date(4)==10){
			lcd.print("09");
		}
		else{
			lcd.print(hourFixed);
		}

		hourFixed = date(4);
		//Рисуем новую часы
		lcd.setTextColor(WHITE);
		lcd.setCursor(24,34);//x,32-true-pos
		//Переместить setCursor в 1 блок if (?)!!!
		if(date(4)<10){
			lcd.print("0");
		}
		lcd.print(date(4));
	}
	if(date(5)!=minuteFixed||printDates==false){
		lcd.setTextSize(4);
		lcd.setTextColor(BLACK);
		lcd.setCursor(24,67);//x,66-true-pos
		//width symbols(2) == 44px
		if(date(5)<10&&date(5)!=0){
			lcd.setCursor(48,67);
			/*На циферблате есть баг
			когда число 99 перекрывает
			какую-то часть месяца, поэтому
			блок с отображением месяца
			стоит в конце. стоит учесть*/
			if(minuteFixed!=99){
				lcd.print(minuteFixed);
			}
		}
		else if(date(5)==10){
			lcd.print("09");
		}
		else{
			lcd.print(minuteFixed);
		}

		minuteFixed = date(5);
		//Рисуем новые минуты
		lcd.setTextColor(WHITE);
		lcd.setCursor(24,67);
		if(date(5)<10){
			lcd.print("0");
		}
		lcd.print(date(5));
		printDates=true;
	}
}

void setup(){
	//initialization timer for clock
	MsTimer2::set(993, timerSeconds);//993-12mHz OR 16mHz i dont know
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

	}
	seconds=conv2d(__TIME__ + 6)+8;
	minutes=conv2d(__TIME__ + 3);
	hours=conv2d(__TIME__);
	day=29;
	numWeekDay=7;
	month=11;
	year=2015;
}

void loop(){
	if(MenuType[MenuNowPos]==0){

	}
}
