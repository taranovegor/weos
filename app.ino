/*
weOS ROM 0.4.0
Board: Arduino UNO
LCD: ILI9163C 1.44" 128x128
время до очищения дисплея добавить!!!
*/
#include <SPI.h>
#include <MsTimer2.h>
#include <EEPROM.h>
#include <MemoryFree.h>
#include <TFT_ILI9163C.h>
#include "_fonts/defaultFont.c"
/*Bluetooth
#include <HC05.h>

#ifdef HC05_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
HC05 btSerial = HC05(A2, A5, A3, A4);  // cmd, state, rx, tx
#else
HC05 btSerial = HC05(3, 2);  // cmd, state
#endif
*/

/*Analog Read начало*/
//Исправление для более быстрой работы
//http://geektimes.ru/post/255744/
#define FASTADC 1
// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
/*Analog Read конец*/

/*Цвета*/
#define	BLACK 0x0000
#define	RED   0xF800
#define WHITE 0xFFFF

/*Выходы*/
//Кнопки (analog)
#define back 1
#define ok 2
#define up 3
#define down 4
//Прочее (digital ШИМ)
#define vibration 3
#define backlight 9
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

/*Переменные*/
/*Для работы с таймерами (?long?)*/
unsigned int currentTime;
unsigned int loopTime;
/*Для работы с временем, датой*/
volatile byte seconds;
byte minutes;
byte hours;
byte day;
byte numWeekDay;
byte month;
unsigned int year;
/*Для работы с циферблатом, фикс даты*/
byte minuteFixed = 0;
byte hourFixed = 0;
byte dayFixed = 0;
/*Для отображения дат, месяца в буквах (хех)*/
const char* namesDays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char* namesMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const byte daysinMonths[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
/*Для работы с будильником*/
//unsigned int vibrationCycle;
const byte vibrationMode[] = {100, 125, 150, 175, 200, 225, 250};

/*Переменные для работы с EEPROM (память)*/
/*Адреса*/
//Дата
const byte minuteAddress = 1;
const byte hourAddress = 2;
const byte dayAddress = 3;
const byte numWeekDayAddress = 4;
const byte monthAddress = 5;
const byte yearAddress = 6;
//Циферблат
const byte clockFaceTypeAddress = 7;
//const int noname reserv = 8;
//Экран
const byte brightnessAddress = 9;
const byte backlightTimerAddress = 10;
//const int noname reserv = 11;
//const int noname reserv = 12;
//Будильник (AC-AlarmClock)
const byte ACMinute = 13;
const byte ACHour = 14;
const byte ACDay = 15;
const byte ACDaysWeek = 16;
const byte ACRepetition = 17;
//const int noname reserv = 18;
/*Байты для сохранения данных EEPROM*/
//...
//Экран
byte brightness;
byte backlightTimer;
//...
/*Для работы с настройками*/
byte settingStep;
byte currentPer;
int fixedSetNum;//Фиксированная цифра настроек (для удаление прошлой)

/*Прочее*/
boolean devMode = false;//Деволперский режим
boolean printDates = false;//Спец.переменная для работы с датами
boolean sleepMode = false;//Режим сна
boolean renderingStatics = false;//Отрисовка статических изображений


/*Переменные для работы меню/c меню*/
char* MenuName[14];//Имя меню
//MenuType = 0;// - циферблат
//MenuType = 1;// - стандартное меню с дочерними элементами
//MenuType = 2;// - элементы БЕЗ дочерних элементов
unsigned int MenuType[14];//Тип меню
unsigned int MenuParent[14];//Родитель меню
unsigned int MenuChildFirst[14];//Первый потомок
unsigned int MenuChildLast[14];//Последний потомок
/**Переменные для работы навигации в меню**/
unsigned int MenuLevel = 0;//Уровень меню (от ok, back)
byte MenuCurPos = 0;//Текущее положение курсора (от up, down)
//unsigned byte MenuPrevPos = 0;//Текущее положение курсора (от up, down)

/*Заполнение массива меню*/
void MenuSetup(){
	/*
	MenuName[]="";
	MenuType[]=;
	MenuParent[]=;
	MenuChildFirst[]=;
	MenuChildLast[]=;
	*/
	//Циферблат
	MenuName[0]="";
	MenuType[0]=0;
	MenuParent[0]=0;
	MenuChildFirst[0]=1;
	MenuChildLast[0]=1;

	MenuName[1]="Menu";
	MenuType[1]=1;
	MenuParent[1]=0;
	MenuChildFirst[1]=2;
	MenuChildLast[1]=5;

	MenuName[2]="Clock";
	MenuType[2]=1;
	MenuParent[2]=1;
	MenuChildFirst[2]=6;
	MenuChildLast[2]=8;

	MenuName[3]="Screen";
	MenuType[3]=1;
	MenuParent[3]=1;
	MenuChildFirst[3]=10;
	MenuChildLast[3]=11;

	MenuName[4]="Bluetooth";
	MenuType[4]=1;
	MenuParent[4]=1;
	MenuChildFirst[4]=0;
	MenuChildLast[4]=0;

	MenuName[5]="Device";
	MenuType[5]=1;
	MenuParent[5]=1;
	MenuChildFirst[5]=12;
	MenuChildLast[5]=13;

	MenuName[6]="Alarm";
	MenuType[6]=1;
	MenuParent[6]=2;
	MenuChildFirst[6]=0;
	MenuChildLast[6]=0;

	MenuName[7]="Time";
	MenuType[7]=5;
	MenuParent[7]=2;
	MenuChildFirst[7]=0;
	MenuChildLast[7]=0;

	MenuName[8]="Date";
	MenuType[8]=6;
	MenuParent[8]=2;
	MenuChildFirst[8]=0;
	MenuChildLast[8]=0;

	MenuName[9]="//Reserv";
	MenuType[9]=1;
	MenuParent[9]=2;
	MenuChildFirst[9]=0;
	MenuChildLast[9]=0;

	MenuName[10]="Brightness";
	MenuType[10]=4;
	MenuParent[10]=3;
	MenuChildFirst[10]=0;
	MenuChildLast[10]=0;

	MenuName[11]="Backlight";
	MenuType[11]=4;
	MenuParent[11]=3;
	//MenuChildFirs1110]=;
	//MenuChildLas1110]=;

	MenuName[12]="Reboot";
	MenuType[12]=2;
	MenuParent[12]=5;
	MenuChildFirst[12]=0;
	MenuChildLast[12]=0;

	MenuName[13]="Information";
	MenuType[13]=2;
	MenuParent[13]=5;
	MenuChildFirst[13]=0;
	MenuChildLast[13]=0;
}

boolean pressed(byte button){
	if(analogRead(button)==0){
		return true;
	}
	else{
		return false;
	}
}

void DrawMenu(){
	lcd.setTextSize(2);
	byte menuCursorPos=1;//Позиция курсора в принте меню
	for(int i=MenuChildFirst[MenuLevel];i<MenuChildLast[MenuLevel]+1;i++){
		lcd.setCursor(3,menuCursorPos);
		menuCursorPos=menuCursorPos+16;//Если надумаю ставить название меню просто поменять принт и сложение местами
		lcd.print(MenuName[i]);
	}
	lcd.drawFastVLine(0,MenuCurPos*16+3,14,WHITE);
}

void BandSettings(){
	currentPer=brightness*100/240;
	if(renderingStatics==false){
		lcd.setTextSize(2);
		lcd.setCursor(8,3);
		lcd.print("Brightness");
		lcd.fillRect(4, 69, 1.2*currentPer, 10, WHITE);
		renderingStatics=true;
	}
	if(currentPer!=fixedSetNum){
		lcd.setTextColor(BLACK);
		if(fixedSetNum==100){
			lcd.setCursor(46,45);
		}
		else if(fixedSetNum<10){
			lcd.setCursor(58,45);
		}
		else{
			lcd.setCursor(52,45);
		}
		lcd.print(fixedSetNum);
		if(currentPer<fixedSetNum){
			lcd.fillRect(1.2*currentPer+6, 69, 6, 10, BLACK);//ТЕСТ скобки
		}
		else if(currentPer>fixedSetNum){
			lcd.fillRect(1.2*currentPer-2, 69, 6, 10, WHITE);
		}
		fixedSetNum=currentPer;

	}
	lcd.setTextColor(WHITE);
	if(currentPer==100){
		lcd.setCursor(46,45);
	}
	else if(currentPer<10){
		lcd.setCursor(58,45);
	}
	else{
		lcd.setCursor(52,45);
	}
	lcd.print(currentPer);
}

void TimeSettings(){
	lcd.setTextSize(2);
	lcd.setTextColor(WHITE);
	lcd.setCursor(42,3);
	lcd.print("Time");
	//main
	lcd.setTextSize(3);
	lcd.setCursor(26,50);
	if(time(4)<10){
		lcd.print("0");
	}
	lcd.print(time(4));
	lcd.print(":");
	if(time(5)<10){
		lcd.print("0");
	}
	lcd.print(time(5));
	switch(settingStep){
		case 0:
			lcd.drawFastHLine(26, 76, 33, WHITE);
			break;
		case 1:
			lcd.drawFastHLine(68, 76, 33, WHITE);
			break;
	}
}

void DateSettings(){
	lcd.setTextSize(2);
	lcd.setTextColor(WHITE);
	if(renderingStatics==false){
		lcd.setCursor(42,3);
		lcd.print("Date");
		renderingStatics=true;
	}
	//День месяца
	lcd.setCursor(10,56);
	lcd.print(namesDays[time(3)-1]);
	//Дата
	lcd.setCursor(52,56);
	if(time(2)<10){
		lcd.print("0");
	}
	lcd.print(time(2));
	//Месяц
	lcd.setCursor(84,56);
	lcd.print(namesMonths[time(1)-1]);
	switch(settingStep){
		case 0:
			lcd.drawFastHLine(10, 74, 34, WHITE);
			break;
		case 1:
			lcd.drawFastHLine(52, 74, 22, WHITE);
			break;

		case 2:
			lcd.drawFastHLine(84, 74, 34, WHITE);
			break;
	}
}

/*Установка даты и времени от компьютера*/
/*static uint8_t conv2d(const char* p){
	uint8_t v =0;
	if('0' <= *p && *p <= '9') v = *p - '0';
	return 10 * v + *++p - '0';
}*/

/*Добавление секунд (для MsTimer2)*/
void timerSeconds() {
	seconds++;
}

/*Передача, подсчёт дат*/
unsigned int time(byte arg){
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
	//Готовим (на печи) строку для возращения
	//year = 0 months = 1 days = 2 numWeekDay = 3 hours = 4 minutes = 5 seconds = 6
	switch(arg){
		case 0:
			return year;
		case 1:
			return month;
		case 2:
			return day;
		case 3:
			return numWeekDay;
		case 4:
			return hours;
		case 5:
			return minutes;
		case 6:
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

/*Будильник
void AlarmClock(){
	if(time(4)==0&&time(5)==1){
		if(currentTime>=loopTime+10){
			loopTime=currentTime;
			vibrationCycle++;
			if(vibrationCycle>7){
				vibrationCycle=0;
			}
			analogWrite(5, vibrationMode[vibrationCycle-1]);
		}
	}
	else{
		analogWrite(5, 0);
	}
}

/*Энерго Сберегающий режим*/
void powerSaveMode(){

}

/*Функция рестарта*/
void(* resetFunc) (void) = 0;

void setup(){
	/*Таймер часов*/
	MsTimer2::set(993, timerSeconds);//993-16mHz
	MsTimer2::start();
	/*Serial порт*/
	//Serial.begin(9600);
	/*чтение EEPROM*/
	brightness=EEPROM.read(brightnessAddress);
	//backlightTimer=EEPROM.read(backlightTimer)*60*10*10;
	/*Инциализация экрана*/
	lcd.begin();
	lcd.setFont(&defaultFont);
	/*Установка типа пинов*/
	//Аналоговые
	pinMode(A1, INPUT);
	pinMode(A2, INPUT);
	pinMode(A3, INPUT);
	pinMode(A4, INPUT);
	//Цифровые
	pinMode(backlight, OUTPUT);
	pinMode(vibration, OUTPUT);
	//
	analogWrite(backlight, brightness);
	/*DevMode включчение при зажатии кнопок back и ok*/
	if(analogRead(back)==0&&digitalRead(ok)==0){
		devMode = true;
	}
	/*Инциализация меню*/
	MenuSetup();
	/*Установка времени (только PC и COM-port)*/
	//seconds=conv2d(__TIME__ + 6)+6;
	//minutes=conv2d(__TIME__ + 3);
	//hours=conv2d(__TIME__);
	seconds=0;
	minutes=EEPROM.read(minuteAddress);
	hours=EEPROM.read(hourAddress);
	day=EEPROM.read(dayAddress);
	numWeekDay=EEPROM.read(numWeekDayAddress);
	month=EEPROM.read(monthAddress);//7[proverka]
	year=2015;
	/*Установка fix-дат*/
	minuteFixed=time(5);
	hourFixed=time(4);
	dayFixed=time(2);//раньше не стояла, нужно проверить
	/*Установка значений для таймеров*/
	currentTime = millis()/100;
	/*1sec = 1000ms
	1000/10=10ps
	5-пол секунды
	10-секунда*/
	loopTime = currentTime;
}

void loop(){
	//Обновляем текущее время
	currentTime = millis()/100;
	//Затрагиваем функцию time для обновления, так как иногда видимо в меню пропускается
	//AlarmClock();
	//Выход из меню, если небыло действий на протяжении 600мс
	if(currentTime>=loopTime+600&&MenuLevel>0){
		loopTime=currentTime;
		printDates=false;
		MenuLevel=0;
		MenuCurPos=0;
		switch(MenuType[MenuLevel]){
			case 4:
				brightness=EEPROM.read(brightnessAddress);
				break;
		}
		lcd.clearScreen();
	}
	//Действие при нажатии на ok/back
	if(pressed(ok)&&currentTime>=loopTime+5){
		loopTime=currentTime;
		renderingStatics=false;
		switch(MenuType[MenuLevel]){
			case 4:
				EEPROM.write(brightnessAddress, brightness);
				MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
				MenuLevel=MenuParent[MenuLevel];
				break;
			case 5:
				settingStep++;
				if(settingStep>1){
					EEPROM.write(minuteAddress, minutes);//День недели
					EEPROM.write(hourAddress, hours);//День месяца
					settingStep=0;
					MenuLevel=MenuParent[MenuLevel];
				}
				break;
			case 6:
				settingStep++;
				if(settingStep>2){
					EEPROM.write(numWeekDayAddress, numWeekDay);//День недели
					EEPROM.write(dayAddress, day);//День месяца
					EEPROM.write(monthAddress, month);//Mесяц
					settingStep=0;
					MenuLevel=MenuParent[MenuLevel];
				}
				break;
			default:
				MenuLevel=MenuChildFirst[MenuLevel]+MenuCurPos;
				MenuCurPos=0;
				break;
		}
		lcd.clearScreen();
	}
	if(analogRead(back)==0&&MenuLevel!=0&&currentTime>=loopTime+5){
		loopTime=currentTime;
		renderingStatics=false;
		switch(MenuType[MenuLevel]){
			case 5:
				settingStep--;
				if(settingStep<0){
					settingStep=0;
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
				}
			case 6:
				settingStep--;
				if(settingStep<0){
					settingStep=0;
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
				}
				break;
			default:
				MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
				MenuLevel=MenuParent[MenuLevel];
				if(MenuLevel==0){//Если после обновления menulevel = 0, тогда обн. принт
					printDates=false;
				}
				break;
		}
		lcd.clearScreen();
	}
	//Действие при нажатии на up/down
	if(analogRead(up)==0&&currentTime>=loopTime+5){
		loopTime=currentTime;
		switch(MenuType[MenuLevel]){
			case 1:
				MenuCurPos--;
				if(MenuCurPos==255){
					MenuCurPos=MenuChildLast[MenuLevel]-MenuChildFirst[MenuLevel];
				}
				lcd.drawFastVLine(0,0,128,BLACK);
				break;
			case 4:
				brightness+=12;
				if(brightness>240){
					brightness=240;
				}
				fixedSetNum=currentPer;
				//lcd.fillRect(4, 69, 120, 10, BLACK);
				analogWrite(backlight, brightness);
				break;
			case 5:
				switch(settingStep){
					case 0:
						fixedSetNum=hours;
						hours++;
						if(hours>23){
							hours=0;
						}
						lcd.setTextColor(BLACK);
						lcd.setCursor(26,50);
						if(fixedSetNum<10){
							lcd.print("0");
						}
						lcd.print(fixedSetNum);
						fixedSetNum=hours;
						break;
					case 1:
						fixedSetNum=minutes;
						minutes++;
						if(minutes>59){
							minutes=0;
						}
						lcd.setTextColor(BLACK);
						lcd.setCursor(68,50);
						if(fixedSetNum<10){
							lcd.print("0");
						}
						lcd.print(fixedSetNum);
						fixedSetNum=minutes;
						break;
				}
				break;
			case 6:
				switch(settingStep){
					case 0:
						fixedSetNum=numWeekDay;
						numWeekDay++;
						lcd.setTextColor(BLACK);
						lcd.setCursor(10,56);
						lcd.print(namesDays[fixedSetNum-1]);
						fixedSetNum=numWeekDay;
						break;
					case 1:
						fixedSetNum=day;
						day++;
						if(day==32){
							month--;
						}
						lcd.setTextColor(BLACK);
						lcd.setCursor(52,56);
						if(fixedSetNum<10){
							lcd.print("0");
						}
						lcd.print(fixedSetNum);
						fixedSetNum=day;
						break;
					case 2:
						fixedSetNum=month;
						month++;
						lcd.setTextColor(BLACK);
						lcd.setCursor(84,56);
						lcd.print(namesMonths[fixedSetNum-1]);
						fixedSetNum=month;
						break;
				}
				break;
		}
	}
	if(analogRead(down)==0&&currentTime>=loopTime+5){
		loopTime=currentTime;
		switch(MenuType[MenuLevel]){
			case 1:
				MenuCurPos++;
				lcd.drawFastVLine(0,0,128,BLACK);
				if(MenuCurPos>MenuChildLast[MenuLevel]-MenuChildFirst[MenuLevel]){
					MenuCurPos=0;
				}
				break;
			case 4:
				brightness-=12;
				if(brightness<12){
					brightness=12;
				}
				fixedSetNum=currentPer;
				//lcd.fillRect(4, 69, 120, 10, BLACK);
				analogWrite(backlight, brightness);
				break;
			case 5:
				switch(settingStep){
					case 0:
						fixedSetNum=hours;
						hours--;
						if(hours==255){
							hours=23;
						}
						lcd.setTextColor(BLACK);
						lcd.setCursor(26,50);
						if(fixedSetNum<10){
							lcd.print("0");
						}
						lcd.print(fixedSetNum);
						fixedSetNum=hours;
						break;
					case 1:
						fixedSetNum=minutes;
						minutes--;
						if(minutes==255){
							minutes=59;
						}
						lcd.setTextColor(BLACK);
						lcd.setCursor(68,50);
						if(fixedSetNum<10){
							lcd.print("0");
						}
						lcd.print(fixedSetNum);
						fixedSetNum=minutes;
						break;
				}
				break;
			case 6:
				switch(settingStep){
					case 0:
						fixedSetNum=numWeekDay;
						numWeekDay--;
						if(numWeekDay<1){
							numWeekDay=7;
						}
						lcd.setTextColor(BLACK);
						lcd.setCursor(10,56);
						lcd.print(namesDays[fixedSetNum-1]);
						fixedSetNum=numWeekDay;
						break;
					case 1:
						fixedSetNum=day;
						day--;
						if(day<1){
							day=daysinMonths[month];
						}
						lcd.setTextColor(BLACK);
						lcd.setCursor(52,56);
						if(fixedSetNum<10){
							lcd.print("0");
						}
						lcd.print(fixedSetNum);
						fixedSetNum=day;
						break;
					case 2:
						fixedSetNum=month;
						month--;
						if(month<1){
							month=12;
						}
						lcd.setTextColor(BLACK);
						lcd.setCursor(84,56);
						lcd.print(namesMonths[fixedSetNum-1]);
						fixedSetNum=month;
						break;
				}
				break;
		}
	}
	//
	switch(MenuType[MenuLevel]){
		case 0:
			DigitalClockFace();
			break;
		case 1:
			DrawMenu();
			break;
		case 4:
			BandSettings();
			break;
		case 5:
			seconds=0;
			TimeSettings();
			break;
		case 6:
			DateSettings();
			break;
	}
	switch(MenuLevel){
		case 12:
			analogWrite(backlight, 0);
			resetFunc();
			break;
		case 13:
			lcd.setCursor(3,0);
			lcd.setTextSize(2);
			lcd.print("Information");
			lcd.setCursor(2,16);
			lcd.print("OS version");
			lcd.setCursor(2,32);
			lcd.print("0.4.0 alpha");
			lcd.setCursor(2,48);
			lcd.print("SOC");
			lcd.setCursor(2,64);
			lcd.print("ATmea328p");
			lcd.setCursor(2,80);
			lcd.print("Free RAM");
			lcd.setCursor(2,96);
			lcd.print(freeMemory());
			lcd.print("/2048Kb");
			break;
	}
}
