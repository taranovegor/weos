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
#define back 5
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

boolean test = true;



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

class Menu{
	private:
		byte menuLevel;
		byte menuCursor;
		const char* MenuName[8] =      {"", "", "Menu", "toggle", "pop-up"};
		const byte MenuType[8] =       {0, 1, 1, 2, 3};
		const byte MenuParent[8] =     {0, 0, 1, 2, 2};
		const byte MenuChildFirst[8] = {1, 2, 3, 3, 4};
		const byte MenuChildLast[8] =  {1, 4, 4, 3, 4};
		/*переключатель*/
		void toggle(){
			test=!test;
		}
		/*всплывающее окно*/
		void popup(){

		}
		/*курсор (удалить старый, нарисовать новый)*/
		void drawCursor(){
			lcd.drawFastVLine(0,1,128,BLACK);
			lcd.drawFastVLine(0,menuCursor*16+3,14,WHITE);
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
		}
		/*при нажатии на back*/
		void close(){
			menuLevel=MenuParent[menuLevel];
		}
};

class Menu{
	private:
	byte menuLevel;
	byte menuCursor;
	//                         0   1   2       3         4
	const char* MenuName[8] = {"", "", "Menu", "toggle", "pop-up"};
	//                        0  1  2  3  4
	const byte MenuType[8] = {0, 1, 1, 2, 3};
	//                          0  1  2  3  4
	const byte MenuParent[8] = {0, 0, 1, 2, 2};
	//                              0  1  2  3  4
	const byte MenuChildFirst[8] = {1, 2, 3, 3, 4};
	//                             0  1  2  3  4
	const byte MenuChildLast[8] = {1, 4, 4, 3, 4};
	//
	void toggle(){
		test=!test;
	}

	void popup(){

	}
	void drawCursor(){
		lcd.drawFastVLine(0,1,128,BLACK);
		lcd.drawFastVLine(0,menuCursor*16+3,14,WHITE);
	}
	//
	public:
	//
	void open(){
		//if(MenuType[menulevel]!=у которых потомок = 0) return;
		switch(MenuType[MenuChildFirst[menuLevel]+menuCursor]){
			case 2:
				toggle();
				break;
			case 3:
				popup();
				break;
			default:
				menuLevel=MenuChildFirst[menuLevel]+menuCursor;
				break;
		}
		show();
	}
	//
	void close(){
		//if(MenuType[menulevel]!=у которых потомок = 0) return;
		menuLevel=MenuParent[menuLevel];
	}
	void moveup(){
		menuCursor--;
		menuCursor=limValue(menuCursor, MenuChildLast[menuLevel]-MenuChildFirst[menuLevel]);
		drawCursor();
	}
	void movedown(){
		menuCursor++;
		menuCursor=limValue(menuCursor, MenuChildLast[menuLevel]-MenuChildFirst[menuLevel]);
		drawCursor();
	}
	//
	void show(){
		byte menuCursorPos=1;
		for(byte i=MenuChildFirst[menuLevel]; i<MenuChildLast[menuLevel]+1;i++){
			lcd.setCursor(3,menuCursorPos);
			lcd.print(MenuName[i]);
			switch(MenuType[i]){
				case 2:
					lcd.fillRect(100, menuCursorPos+4, 20, 10, WHITE);
					if(test) lcd.fillRect(111, menuCursorPos+5, 8, 8, GREEN);
					else lcd.fillRect(101, menuCursorPos+5, 8, 8, RED);
					break;
				default:
					break;
			}
			menuCursorPos=menuCursorPos+16;//Если надумаю ставить название меню просто поменять принт и сложение местами
		}
	}
};

class Settings{

};

Menu menu;
Settings settings;

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
	if(pressed(ok)&&millisDelay(TimerButton)) menu.next();
	if(pressed(back)&&millisDelay(TimerButton)) menu.previous();
	if(pressed(up)&&millisDelay(TimerButton)) menu.moveup();
	if(pressed(down)&&millisDelay(TimerButton)) menu.movedown();
}