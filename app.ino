#include <SPI.h>
#include <EEPROM.h>
#include <MemoryFree.h>
#include <TFT_ILI9163C.h>
#include <DS1302.h>
/*Analog Read начало*/
//Исправление для более быстрой работы
//http://geektimes.ru/post/255744/
#define FASTADC 1
// defines for s  etting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
/*Analog Read конец*/

/*Выходы*/
//Кнопки (analog)
#define back 1
#define ok 2
#define up 3//to 3
#define down 4
//
#define backlight 9
/*
---|Buttons pin's
back 1	up 3
ok 2	down 4
VCC GND CS RST A0 SDA SCK LED
---|LCD pin's
__CS - 10	__SDA 11
__DC - 6	__SCK 13
__RST - 8   __LED 9
---|DS1302 pin's |beta
__CE - 7 = RST
__IO - 5 = DATA
__CLK- 4 = SCLK
---|Other pin's
vibration 5
backlight (__LED) 9
*/

/*Установка пинов для экрана*/
#define __CS 10
#define __DC 6
#define __RST 8
TFT_ILI9163C lcd = TFT_ILI9163C(__CS, __DC, __RST);

/*Установка пинов DS1302 */
#define __CE  7 //RST
#define __IO  5 //I/0
#define __CLK 4 //SCLK
DS1302 rtc(__CE, __IO, __CLK);

long currentTime;
long loopTime;
byte TimerButton;

boolean pressed(byte button){//Возращает true если кнопка нажата
	if(analogRead(button)<=50) return true;
	else return false;
}

/*Функция пауз для кнопок*/
boolean millisDelay(byte delay){//Возращает true если таймер нажатия кнопки прошёл
	if(currentTime>=loopTime+delay){
		loopTime=currentTime;
		return true;
	}
	else return false;
}

byte limValue(byte current, byte max, byte min = 0){
	if(current>max&&current!=255) return min;
	else if(current<min||current==255) return max;
	else return current;
}

byte time(byte arg){
	Time t = rtc.time();
	//months = 1 days = 2 numWeekDay = 3 hours = 4 minutes = 5 seconds = 6
	switch(arg){
		case 1: return t.mon;//Месяц
		case 2: return t.date;//Дата
		case 3: return t.day;//День недели
		case 4: return t.hr;//Часы
		case 5:	return t.min;//Минуты
		case 6: return t.sec;//Секунды
		default: break;
	}
}

class Settings{
	public:
		boolean sleepMode = false;
		/**/
};

Settings settings;

class Menu{
	private:
		byte menuLevel;
		byte menuCursor;
		const char* MenuName[8] =      {"", "", "Menu", "toggle", "pop-up"};
		const byte MenuType[8] ={0, 1, 1, 2, 3};
		const byte MenuParent[8] =     {0, 0, 1, 2, 2};
		const byte MenuChildFirst[8] = {1, 2, 3, 3, 4};
		const byte MenuChildLast[8] =  {1, 4, 4, 3, 4};
		/*Переменные для работы с отрисовкой*/
		boolean renderingStatics = false;
		boolean printDates = false;
		/*переключатель*/
		void toggle(){
			//settings.toog=!settings.toog;
		}
		/*всплывающее окно*/
		void popup(){
			lcd.fillRect(12, 12, 102, 102, GREEN);
		}
		/*курсор (удалить старый, нарисовать новый)*/
		void drawCursor(){
			lcd.drawFastVLine(0,1,128,BLACK);
			lcd.drawFastVLine(0,menuCursor*16+3,14,WHITE);//drawRect
		}
	public:
		/*при нажатии на ok*/
		void open(){
			switch(MenuType[MenuChildFirst[menuLevel]+menuCursor]){
				case 2: toggle(); break;
				case 3: popup(); break;
				default: menuLevel=MenuChildFirst[menuLevel]+menuCursor; break;
			}
			show();
			lcd.print(menuLevel);
		}
		/*при нажатии на back*/
		void close(){
			lcd.clearScreen();
			menuLevel=MenuParent[menuLevel];
			menuCursor=0;
			show();
		}
		/*при нажатии на up*/
		void moveup(){
			menuCursor--;
			menuCursor=limValue(menuCursor, MenuChildLast[menuLevel]-MenuChildFirst[menuLevel]);
			drawCursor();
		}
		/*при нажатии на down*/
		void movedown(){
			menuCursor++;
			menuCursor=limValue(menuCursor, MenuChildLast[menuLevel]-MenuChildFirst[menuLevel]);
			drawCursor();
		}
		/*функция рисования меню*/
		void show(){
			//if(menuLevel==0){ DigitalClockFace(); return; }
			lcd.print(struct1[0]);
			byte menuCursorPos=1;
			for(byte i=MenuChildFirst[menuLevel]; i<MenuChildLast[menuLevel]+1;i++){
				lcd.setCursor(3,menuCursorPos);
				lcd.print(MenuName[i]);
				switch(MenuType[i]){
					case 2:
						//lcd.fillRect(100, menuCursorPos+4, 20, 10, WHITE);
						//if(settings.toog) lcd.fillRect(111, menuCursorPos+5, 8, 8, GREEN);
						//else lcd.fillRect(101, menuCursorPos+5, 8, 8, RED);
						break;
				}
				menuCursorPos=menuCursorPos+16;//Если надумаю ставить название меню просто поменять принт и сложение местами
			}
			lcd.drawFastVLine(0,menuCursor*16+3,14,WHITE);
		}
};
//
Menu menu;


void setup(){
	pinMode(backlight, OUTPUT);
	//pinMode(vibration, OUTPUT);//вибромотор вкл/выкл
	//Прочее
	analogReference(DEFAULT);
	pinMode(A0, INPUT);
	pinMode(A1, INPUT);
	pinMode(A2, INPUT);
	pinMode(A3, INPUT);
	pinMode(A4, INPUT);
	lcd.begin();
	analogWrite(backlight, 50);
}

void loop(){
	currentTime = millis()/100;
	TimerButton=2;
	if(pressed(ok)&&millisDelay(TimerButton)) menu.open();
	if(pressed(back)&&millisDelay(TimerButton)) menu.close();
	if(pressed(up)&&millisDelay(TimerButton)) menu.moveup();
	if(pressed(down)&&millisDelay(TimerButton)) menu.movedown();
}
