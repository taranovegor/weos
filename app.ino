
#include <SPI.h>
#include <MsTimer2.h>
#include <TFT_ILI9163C.h>
#include "_fonts/mono_mid.c"

/*Цвета*/
#define	BLACK 0x0000
#define	RED   0xF800//debug
#define WHITE 0xFFFF
/*
VCC GND CS RST A0 SDA SCK LED
---|LCD pin's
__CS - 10	__SDA 11
__DC - 6	__SCK 13
__RST - 8   __LED 9
*/
#define __CS 10
#define __DC 6
#define __RST 8

/*Установка пинов для экрана*/
TFT_ILI9163C lcd = TFT_ILI9163C(__CS, __DC, __RST);

/*Для работы с временем, датой*/
int seconds;
int minutes;
int hours;
unsigned int day;
unsigned int numWeekDay;
unsigned int month;
unsigned int year;
/*Для работы с циферблатом, фикс даты*/
byte minuteFixed = 0;
byte hourFixed = 0;
byte dayFixed = 0;
/*Для отображения дат, месяца в буквах*/
const char* namesDays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char* namesMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const int daysinMonths[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
/*Прочее*/
boolean printDates = false;//Спец.переменная для работы с датами

/*Добавление секунд (для MsTimer2)*/
void timerSeconds() {
	seconds++;
}

/*Передача, подсчёт дат*/
unsigned int time(int arg){
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
//перемещено
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
	//Готовим строку для возращения
	//year = 0 months = 1 days = 2 numWeekDay = 3 hours = 4 minutes = 5 seconds = 6
	if(arg==0){
		return year;
	}
	else if(arg==1){
		return month;
	}
	else if(arg==2){
		return day;
	}
	else if(arg==3){
		return numWeekDay;
	}
	else if(arg==4){
		return hours;
	}
	else if(arg==5){
		return minutes;
	}
	else if(arg==6){
		return seconds;
	}
	//int retstr[] = {year, month, day, numWeekDay, hours, minutes, seconds};
	//Возращаем строку
	//return retstr[arg];
}

void DigitalClockFace(){
	//Правая колонка
	if(time(2)!=dayFixed||printDates==false){
		printDates=false;
		lcd.clearScreen();
		lcd.setTextSize(2);
		//lcd.setTextColor(WHITE);
		//День недели
		lcd.setCursor(70,32);
		lcd.print(namesDays[time(3)-1]);
		//День месяца
		lcd.setCursor(70,48);
		lcd.print(time(2));
		//Месяц
		lcd.setCursor(70,64);
		lcd.print(namesMonths[time(1)-1]);
		//Обновляем переменную
		dayFixed=time(2);
	}
	//Часы
	if(time(4)!=hourFixed||printDates==false){
		//
		lcd.setTextSize(4);
		lcd.setTextColor(BLACK);
		lcd.setCursor(24,30);
		if(time(4)<10&&time(4)!=0){
			lcd.setCursor(48,30);
			lcd.print(hourFixed);
		}
		else if(time(4)==10){
			lcd.print("09");
		}
		else{
			lcd.print(hourFixed);
		}
		hourFixed=time(4);
		lcd.setTextColor(WHITE);
		lcd.setCursor(24,30);
		if(time(4)<10){
			lcd.print("0");
		}
		lcd.print(time(4));
	}
	//Минуты
	if(time(5)!=minuteFixed||printDates==false){
		lcd.setTextSize(4);
		lcd.setTextColor(BLACK);
		lcd.setCursor(24,63);
		if(time(5)<10&&time(5)!=0){
			lcd.setCursor(48,63);
			lcd.print(minuteFixed);
		}
		else if(time(5)==10){
			lcd.print("09");
		}
		else{
			lcd.print(minuteFixed);
		}
		minuteFixed=time(5);
		lcd.setTextColor(WHITE);
		lcd.setCursor(24,63);
		if(time(5)<10){
			lcd.print("0");
		}
		lcd.print(time(5));
		if(printDates==false){
			printDates=true;
		}
	}
}

void setup(){
	/*Таймер часов*/
	MsTimer2::set(993, timerSeconds);//993-16mHz
	MsTimer2::start();
	/*чтение EEPROM*/
	analogWrite(9, 50);//backlight
	//backlightTimer=EEPROM.read(backlightTimer)*60*10*10;
	/*Инциализация экрана*/
	lcd.begin();
	lcd.setFont(&mono_mid);

	seconds=50;
	minutes=59;
	hours=23;
	hours = 23;
	day=3;
	numWeekDay=3;
	month=3;
	year=2015;
	/*fix-dates*/
	minuteFixed=time(5);
	hourFixed=time(4);
	dayFixed=time(2);
}

void loop(){
	DigitalClockFace();
}
