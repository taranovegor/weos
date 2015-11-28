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

unsigned int minuteFixed = 99;
unsigned int hourFixed = 99;

boolean printDates = false;

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

void setup(){
	//initialization timer for clock
	MsTimer2::set(993, timerSeconds);//993-12mHz
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
	seconds=55;
	minutes=28;
	hours=18;
	day=28;
	numWeekDay=6;
	month=12;
	year=2015;
}

void loop(){

	/*lcd.setTextColor(WHITE);
	lcd.setTextSize(4);
	lcd.setCursor(16,30);//x,32
	lcd.print("22");
	lcd.setCursor(16,64);//x,66
	lcd.print("15");
	//24
	lcd.setTextSize(2);
	lcd.setCursor(62,30);
	lcd.print(namesDays[date(3)-1]);//14
	lcd.setCursor(62,46);
	lcd.print(date(2));
	lcd.setCursor(62,62);
	lcd.print(namesMonths[date(1)-1]);*/

	if(date(4)==0&&date(5)==0&&date(6)==0&&printDates==true){
		printDates=false;
	}
	if(date(4)!=hourFixed){
		lcd.setTextSize(4);
		lcd.setTextColor(BLACK);

		lcd.setCursor(16,32);//x,32-true-pos
		if(date(4)<10&&date(4)!=0){
			lcd.setCursor(40,32);
			lcd.print(hourFixed);
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
		lcd.setCursor(16,32);//x,32-true-pos
		if(date(4)<10){
			lcd.print("0");
		}
		lcd.print(date(4));
	}
	if(date(5)!=minuteFixed){
		lcd.setTextSize(4);
		lcd.setTextColor(BLACK);
		lcd.setCursor(16,66);//x,66-true-pos

		if(date(5)<10&&date(5)!=0){
			lcd.setCursor(40,66);
			/*На циферблате есть баг
			когда число 99 перекрывает
			какую-то часть месяца, поэтому
			блок с отображением месяца
			стоит в конце. стоит учесть*/
			lcd.print(minuteFixed);
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
		lcd.setCursor(16,66);
		if(date(5)<10){
			lcd.print("0");
		}
		lcd.print(date(5));
	}
	if(printDates==false){
		lcd.setTextSize(2);
		lcd.setCursor(62,32);
		lcd.print(namesDays[date(3)-1]);
		lcd.setCursor(62,48);
		lcd.print(date(2));
		lcd.setCursor(62,64);
		lcd.print(namesMonths[date(1)-1]);
		/*
		lcd.drawRoundRect(62,82,10,10,5,WHITE);
		lcd.drawFastHLine(64,86,3,WHITE);
		lcd.drawFastVLine(67,83,4,WHITE);
		//Левое ухо
		lcd.drawLine(61,93-10,64,80, WHITE);
		//Правое ухо
		lcd.drawLine(69,90-10,72,83,WHITE);
		//Левая ножка
		lcd.drawLine(61,93,64,90, WHITE);
		//Правая ножка x1 y1 x2 y2 -6x1x2
		lcd.drawLine(69,90,72,93,WHITE);
		*/
		/*lcd.drawRoundRect(64,82,10,10,5,WHITE);
		lcd.drawFastHLine(66,86,3,WHITE);
		lcd.drawFastVLine(69,83,4,WHITE);
		//x1 y1 x2 y2
		//Левое ухо
		lcd.drawLine(63,93-10,66,80, WHITE);
		//Правое ухо
		lcd.drawLine(71,90-10,74,83,WHITE);
		//Левая ножка
		lcd.drawLine(63,93,66,90, WHITE);
		//Правая ножка
		lcd.drawLine(71,90,74,93,WHITE);*/
		printDates=true;
	}
	//
	//	*
	// *  *
	//*******
}
