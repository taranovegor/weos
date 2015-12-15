#include <SPI.h>
#include <TFT_ILI9163C.h>
#include <MsTimer2.h>
#include <EEPROM.h>
#include "_fonts/defaultFont.c"
//#include <MemoryFree.h>

// Color's
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
//#define	GREEN   0x07E0
//#define CYAN    0x07FF
//#define MAGENTA 0xF81F
//#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Pin's
//buttons
#define down 5
#define up 7
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

//Установка пинов для экрана
TFT_ILI9163C lcd = TFT_ILI9163C(__CS, __DC, __RST);

/*Переменные*/
/**Для работы с таймерами**/
long currentTime;
long loopTime;
/**Для работы с временем, датой**/
unsigned int seconds;
unsigned int minutes;
unsigned int hours;
unsigned int day;
unsigned int numWeekDay;
unsigned int month;
unsigned int year;
/**Для работы с циферблатом**/
unsigned int minuteFixed = 0;
unsigned int hourFixed = 0;
unsigned int dayFixed = 0;
/**Для отображения дат, месяца**/
const char* namesDays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char* namesMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/**Переменные для работы с EEPROM**/
//Адресса EEPROM
unsigned int brightnessAddress = 20;//Адресс яркости экрана
unsigned int backlightTimerAddress = 21;//Адресс таймера подс.
//Байты, если нужно сохранить данные
byte brightness;//Яркость экрана по дефолту, задаётся юзером
/**Для работы с экраном**/
//таймер подсветки
unsigned int backlightTimer;

/**Прочее**/
boolean devMode = false;//Деволперский режим
boolean printDates = false;//Спец.переменная для работы с датами
boolean sleepMode = false;

/**Переменные для работы меню**/
char* MenuName[14];//Имя меню
//MenuType = 0;// - циферблат
//MenuType = 1;// - меню
int MenuType[14];//Тип меню
int MenuParent[14];//Родитель меню
int MenuChildFirst[14];//Первый потомок
int MenuChildLast[14];//Последний потомок
/**Переменные для работы навигации в меню**/
unsigned int MenuLevel = 0;//Уровень меню (от ok, back)
unsigned int MenuCurPos = 0;//Текущее положение курсора (от up, down)
unsigned int MenuPrevPos = 0;//Текущее положение курсора (от up, down)

/*Функции*/

/**Заполнение массива меню**/
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
	MenuChildFirst[3]=0;
	MenuChildLast[3]=0;

	MenuName[4]="Bluetooth";
	MenuType[4]=1;
	MenuParent[4]=1;
	MenuChildFirst[4]=0;
	MenuChildLast[4]=0;

	MenuName[5]="Device";
	MenuType[5]=1;
	MenuParent[5]=1;
	MenuChildFirst[5]=9;
	MenuChildLast[5]=10;

	MenuName[6]="Alarm";
	MenuType[6]=1;
	MenuParent[6]=2;
	MenuChildFirst[6]=0;
	MenuChildLast[6]=0;

	MenuName[7]="Time";
	MenuType[7]=1;
	MenuParent[7]=2;
	MenuChildFirst[7]=0;
	MenuChildLast[7]=0;

	MenuName[8]="//Reserv";
	MenuType[8]=1;
	MenuParent[8]=2;
	MenuChildFirst[8]=0;
	MenuChildLast[8]=0;

	MenuName[9]="Reboot";
	MenuType[9]=1;
	MenuParent[9]=5;
	MenuChildFirst[9]=0;
	MenuChildLast[9]=0;

	MenuName[10]="Information";
	MenuType[10]=1;
	MenuParent[10]=5;
	MenuChildFirst[10]=0;
	MenuChildLast[10]=0;
}

/**Рисование меню**/
void DrawMenu(){
	lcd.setTextSize(2);
	int menuCursorPos=1;//Позиция курсора в принте меню
	for(int i=MenuChildFirst[MenuLevel];i<MenuChildLast[MenuLevel]+1;i++){
		lcd.setCursor(3,menuCursorPos);
		menuCursorPos=menuCursorPos+16;//Если надумаю ставить название меню просто поменять принт и сложение местами
		lcd.print(MenuName[i]);
	}
	lcd.drawFastVLine(0,MenuCurPos*16-16,16,BLACK);
	lcd.drawFastVLine(0,MenuCurPos*16+16,16,BLACK);
	lcd.drawFastVLine(0,MenuCurPos*16,16,WHITE);
}

/**Добавление секунд (для MsTimer2)**/
void timerSeconds() {
	seconds++;
}

/**Установка времени**/
static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9') v = *p - '0';
  return 10 * v + *++p - '0';
}

/**Передача, подсчёт дат**/
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

/**Рисование циферблата**/
void DigitalClockFace(){
	if(date(2)!=dayFixed||printDates==false){
		lcd.clearScreen();
		lcd.setTextSize(2);
		//День недели
		lcd.setCursor(70,34);
		lcd.print(namesDays[date(3)-1]);
		//День месяца
		lcd.setCursor(70,50);
		lcd.print(date(2));
		//Месяц
		lcd.setCursor(70,66);
		lcd.print(namesMonths[date(1)-1]);
		//Обновляем переменную
		dayFixed=date(2);
	}
	if(date(4)!=hourFixed||printDates==false){
		//Устанавливаем размер текста 4 для даты
		lcd.setTextSize(4);

		lcd.setTextColor(BLACK);

		lcd.setCursor(24,34);
		if(date(4)<10&&date(4)!=0){
			/*Если дата(часы) меньше 10,
			2 (x) блоке |1x| меняем удаляем
			прошлое значение
			@Ноль не обновляется!!!*/
			lcd.setCursor(48,34);
			lcd.print(hourFixed);
		}
		else if(date(4)==10){
			/*Если дата(часы) равна 10,
			рисуем 09, так как так легче
			очистить блок 1 и 2
			@Ноль удаляется!!!*/
			lcd.print("09");
		}
		else{
			/*Если дата(часы) равна остальным значениям
			то рисуем прошлое значение в двух блоках
			@Ноль не существует в 1 блоке!!!*/
			lcd.print(hourFixed);
		}
		//Обновляем переменную
		hourFixed = date(4);
		lcd.setTextColor(WHITE);
		lcd.setCursor(24,34);
		if(date(4)<10){
			lcd.print("0");
		}
		lcd.print(date(4));
	}
	if(date(5)!=minuteFixed||printDates==false){
		//Устанавливаем размер текста 4 для даты
		lcd.setTextSize(4);

		lcd.setTextColor(BLACK);

		lcd.setCursor(24,67);
		if(date(5)<10&&date(5)!=0){
			/*Если дата(минуты) меньше 10,
			2 (x) блоке |1x| меняем удаляем
			прошлое значение
			@Ноль не обновляется!!!*/
			lcd.setCursor(48,67);
			lcd.print(minuteFixed);
		}
		else if(date(5)==10){
			/*Если дата(минуты) равна 10,
			рисуем 09, так как так легче
			очистить блок 1 и 2
			@Ноль удаляется!!!*/
			lcd.print("09");
		}
		else{
			/*Если дата(минуты) равна остальным значениям
			то рисуем прошлое значение в двух блоках
			@Ноль не существует в 1 блоке!!!*/
			lcd.print(minuteFixed);
		}
		//Обновляем переменную
		minuteFixed = date(5);
		lcd.setTextColor(WHITE);
		lcd.setCursor(24,67);
		if(date(5)<10){
			lcd.print("0");
		}
		lcd.print(date(5));
		printDates=true;
	}
}

/**Функция рестарта**/
void(* resetFunc) (void) = 0;

void setup(){
	//Таймер часов
	MsTimer2::set(993, timerSeconds);//993-16mHz
	MsTimer2::start();
	//EEPROM, чтение
	//EEPROM.write(backlightTimer, 1);
	brightness=EEPROM.read(brightnessAddress);
	backlightTimer=EEPROM.read(backlightTimer)*60*10*10;
	//Инциализация экрана
	lcd.begin();
	lcd.setFont(&defaultFont);
	analogWrite(backlight, brightness);//Устанавливаем яркость
	//Установка мода пинов
	pinMode(down, OUTPUT);
	pinMode(up, OUTPUT);
	pinMode(ok, OUTPUT);
	pinMode(back, OUTPUT);
	//Статус
	digitalWrite(down, HIGH);
	digitalWrite(up, HIGH);
	digitalWrite(ok, HIGH);
	digitalWrite(back, HIGH);
	//DevMode включение при зажатии кнопки back и ok
	if(digitalRead(back)==LOW&&digitalRead(ok)==LOW){
		devMode = true;
	}
	//Инциализация меню
	MenuSetup();
	//Установка времени (онли PC)
	seconds=conv2d(__TIME__ + 6)+6;
	minutes=conv2d(__TIME__ + 3);
	hours=12;
	//seconds=45;
	//minutes=59;
	//hours=23;
	day=14;
	numWeekDay=1;
	month=12;//7
	year=2015;
	//Установка фикс. дат
	minuteFixed=date(5);
	hourFixed=date(4);
	//dayFixed=date(2);
	//Установка значений для таймеров
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
	//Затрагиваем функцию date для подсчёта, так как иногда теряет секунды в меню
	date(0);//Год
	//Очищение диспления из за бездействия
	if(currentTime>=loopTime+backlightTimer&&sleepMode==false){
		loopTime=currentTime;
		sleepMode=true;
		analogWrite(backlight, 0);
		lcd.sleepMode(true);
	}
	if(digitalRead(ok)==LOW||digitalRead(back)==LOW||digitalRead(up)==LOW||digitalRead(down)==LOW&&sleepMode==true){
		sleepMode=false;
		lcd.sleepMode(false);
		analogWrite(backlight, brightness);
	}


	if(currentTime>=loopTime+600&&MenuLevel>0){
		loopTime=currentTime;
		printDates=false;
		MenuLevel=0;
		MenuCurPos=0;
	}
	if(digitalRead(ok)==LOW&&currentTime>=loopTime+5){
		loopTime=currentTime;
		MenuLevel=MenuChildFirst[MenuLevel]+MenuCurPos;
		lcd.clearScreen();
	}
	if(digitalRead(back)==LOW&&currentTime>=loopTime+5){
		loopTime=currentTime;
		MenuCurPos=MenuPrevPos;
		MenuLevel=MenuParent[MenuLevel];
	}
	if(MenuType[MenuLevel]==0){
		DigitalClockFace();
		lcd.setCursor(0,0);
		lcd.setTextSize(1);
		lcd.print("test");
	}
	else{
		DrawMenu();
	}
	/*if(date(4)>22&&sleepMode==false){
		analogWrite(backlight, 20);
	}
	else if(date(4)<7&&sleepMode==false){
		analogWrite(backlight, 20);
	}
	else{
		analogWrite(backlight, brightness);
	}*/
}
