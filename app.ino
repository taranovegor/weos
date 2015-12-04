#include <SPI.h>
#include <TFT_ILI9163C.h>
#include <MsTimer2.h>
#include <EEPROM.h>
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

TFT_ILI9163C lcd = TFT_ILI9163C(__CS, __DC, __RST);


long currentTime;
long loopTime;
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

char* MenuName[14];

//MenuType = 0;// - циферблат
//MenuType = 1;// - меню
int MenuType[14];

int MenuParent[14];

int MenuChildFirst[14];

int MenuChildLast[14];

unsigned int MenuLevel = 0;
unsigned int MenuCurPos = 0;////MenuCurrentPos

unsigned int backlightAddress = 20;
byte brightness;

void MenuSetup(){
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

	/*MenuName[]="";
	MenuType[]=;
	MenuParent[]=;
	MenuChildFirst[]=;
	MenuChildLast[]=;*/
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

void timerSeconds() {
	seconds++;
}

static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9') v = *p - '0';
  return 10 * v + *++p - '0';
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

void DigitalClockFace(){
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
		lcd.setCursor(24,67);
		if(date(5)<10){
			lcd.print("0");
		}
		lcd.print(date(5));
		printDates=true;
	}
}

/*void printCenter(char* string, int margintop, int size, byte color){
	lcd.setTextColor(color);
	lcd.setTextSize(size);
	lcd.setCursor(((128-(strlen(string)*(size*7)-2))/2)-1,margintop);
	lcd.print(string);
}*/

void(* resetFunc) (void) = 0;

void setup(){
	//initialization timer for clock
	MsTimer2::set(993, timerSeconds);//993-12mHz OR 16mHz i dont know
	MsTimer2::start();

	//EEPROM READ
	//EEPROM.write(backlightAddress, 50);
	brightness = EEPROM.read(backlightAddress);


	lcd.begin();
	analogWrite(backlight, brightness);
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

	MenuSetup();

	seconds=conv2d(__TIME__ + 6)+6;
	minutes=conv2d(__TIME__ + 3);
	hours=conv2d(__TIME__);
	//seconds=45;
	//minutes=29;
	//hours=6;
	day=3;
	numWeekDay=4;
	month=12;
	year=2015;

	currentTime = millis()/100;
	loopTime = currentTime;
	MenuLevel=0;
	minuteFixed=date(5);
	hourFixed=date(4);
}
//lcd.drawCircle(64,42,32,WHITE);

void loop(){
	date(6);
	currentTime = millis()/100;
	if(currentTime>=(loopTime+600)&&MenuLevel>0){
		loopTime=currentTime;
		lcd.clearScreen();
		printDates=false;
		MenuLevel=0;
		MenuCurPos=0;
	}
	if(digitalRead(ok)==LOW&&currentTime>=(loopTime+5)){
		loopTime=currentTime;
		MenuLevel=MenuChildFirst[MenuLevel]+MenuCurPos;
		lcd.clearScreen();
		MenuCurPos=0;
	}
	if(digitalRead(back)==LOW&&currentTime>=(loopTime+5)&&MenuLevel!=0){
		loopTime=currentTime;
		MenuCurPos=MenuParent[MenuLevel];
		MenuLevel=MenuParent[MenuLevel];
		lcd.clearScreen();
		printDates=false;
	}

	if(digitalRead(up)==LOW&&currentTime>=(loopTime+5)&&MenuLevel!=0){
		loopTime=currentTime;
		MenuCurPos--;
	}
	if(digitalRead(down)==LOW&&currentTime>=(loopTime+5)&&MenuLevel!=0){
		loopTime=currentTime;
		MenuCurPos++;
	}
	if(MenuType[MenuLevel]==0){
		/*lcd.setCursor(0,0);
		lcd.setTextSize(1);
		lcd.print(MenuCurPos);*/
		DigitalClockFace();
	}
	if(MenuType[MenuLevel]!=0){
		DrawMenu();
	}
	if(MenuName[MenuLevel]=="Reboot"){
		analogWrite(backlight, 0);
		resetFunc();
	}
	if(MenuName[MenuLevel]=="Information"){
		lcd.print("alo");
	}
	if(date(4)>=22&&date(5)>=30){
		analogWrite(backlight, 10);
	}
	else if(date(4)<=6&&date(5)<30){
		analogWrite(backlight, 10);
	}
	else{
		analogWrite(backlight, 50);
	}
}
