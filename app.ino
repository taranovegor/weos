/*
weOS ROM 0.9
Board: Arduino UNO
LCD: ILI9163C 1.44" 128x128
<cl>DS1302s
0.7 - New FONTS
0.7.1 Fixed layout menu
0.7.2 много тестов.
0.7.3 упорядочены функции, добавлена функция измерения тока (beta)
	millisDelay() переименована на millisDelay()
	изменена ф-я DrawMenu(), теперь элементы меню - статика
	изменена ф-я Alarm(), теперь отложение будильника в последнем блоке if (else if)
0.7.3 - добавлен bluetooth (beta) и комманды
0.8 - bluetooth (stable)
	повышена чуствительность кнопок (в функции переведено значение с 0 на 10)
0.9 - DS
	- добавление DS, изменение структуры.
	- повышена производительность, обновления экрана сведены к минимуму. Интерфейс стал приятнее
</cl>
*/
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
//Прочее (digital)
#define bluetoothPower 2
//Прочее (digital ШИМ)
#define vibration 3//3old
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
#define __CS 10
#define __DC 6
#define __RST 8

#define __CE  7 //RST
#define __IO  5 //I/0
#define __CLK 4 //SCLK


/*Установка пинов для экрана*/
TFT_ILI9163C lcd = TFT_ILI9163C(__CS, __DC, __RST);

/*Установка пинов DS1302 */
DS1302 rtc(__CE, __IO, __CLK);

/*Переменные*/
/*Для работы с таймерами (?long?)*/
long currentTime;
long loopTime;
long voltageTime;
byte TimerButton;

double voltage;

/*Для работы с циферблатом, фикс даты*/
byte minuteFixed = 0;
byte hourFixed = 0;
byte dayFixed = 0;

/*Для отображения дат, месяца в буквах*/
const char* namesDays[8] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char* namesMonths[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const byte daysinMonths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*Для работы с будильником*/
boolean alarmStatus;//Установлен ли будильник true - да, false - нет
byte vibrationCycle;//Цикл вибрации, на ноль есть действие, так что нужно обнулять
const byte vibrationMode[] = {100, 125, 150, 175, 200, 225, 250};
byte alarmMinute;
byte alarmHour;

/*bluetooth*/
boolean bluetoothStatus = false;

/*Переменные для работы с EEPROM (память)*/
/*Адреса*/
//Дата
/*
const byte minuteAddress = 1;
const byte hourAddress = 2;
const byte dayAddress = 3;
const byte numWeekDayAddress = 4;
const byte monthAddress = 5;
const byte yearAddress = 6;
*/
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
//Экран
byte brightness;
byte backlightTimer;

/*Для работы с настройками*/
byte settingStep;
byte currentPer;
byte staticData[3] = {0,0,0};

/*Прочее*/
boolean devMode = false;//Деволперский режим
boolean printDates = false;//Спец.переменная для работы с датами
boolean sleepMode = false;//Режим сна
boolean renderingStatics = false;//Отрисовка статических изображений
boolean serviceWork = false;//Работа какого то сервиса который может выполнятся в момент работы другой программы, например будильник

/*Переменные для работы меню/c меню*/
//Основа построения меню была взята с geektimes.ru
//http://geektimes.ru/post/255408/
char* MenuName[22];//Имя меню
//MenuType = 0;// - циферблат
//MenuType = 1;// - стандартное меню с дочерними элементами
//MenuType = 2;// - элементы БЕЗ дочерних элементов
byte MenuType[22];//Тип меню
byte MenuParent[22];//Родитель меню
byte MenuChildFirst[22];//Первый потомок
byte MenuChildLast[22];//Последний потомок
/**Переменные для работы навигации в меню**/
byte MenuLevel = 0;//Уровень меню (от ok, back)
byte MenuCurPos = 0;//Текущее положение курсора (от up, down)

/*Заполнение меню*/
void MenuSetup(){
	//max-type=8;(status)
	//Циферблат
	MenuName[0]="";
	MenuType[0]=0;
	MenuParent[0]=0;
	MenuChildFirst[0]=1;
	MenuChildLast[0]=1;
	//menu
	MenuName[1]="Menu";
	MenuType[1]=1;
	MenuParent[1]=0;
	MenuChildFirst[1]=2;
	MenuChildLast[1]=6;

	MenuName[2]="Alarm";
	MenuType[2]=1;
	MenuParent[2]=1;
	MenuChildFirst[2]=7;
	MenuChildLast[2]=8;

	MenuName[3]="Clock";
	MenuType[3]=1;
	MenuParent[3]=1;
	MenuChildFirst[3]=10;
	MenuChildLast[3]=11;

	MenuName[4]="Screen";
	MenuType[4]=1;
	MenuParent[4]=1;
	MenuChildFirst[4]=13;
	MenuChildLast[4]=14;

	MenuName[5]="Bluetooth";
	MenuType[5]=1;
	MenuParent[5]=1;
	MenuChildFirst[5]=16;
	MenuChildLast[5]=18;

	MenuName[6]="Device";
	MenuType[6]=1;
	MenuParent[6]=1;
	MenuChildFirst[6]=19;
	MenuChildLast[6]=21;
	//Alarm child 7-9
	MenuName[7]="Status";
	MenuType[7]=8;
	MenuParent[7]=2;
	MenuChildFirst[7]=0;
	MenuChildLast[7]=0;

	MenuName[8]="Time";
	MenuType[8]=7;
	MenuParent[8]=2;
	MenuChildFirst[8]=0;
	MenuChildLast[8]=0;

	//MenuName[9]="Date";
	//MenuType[9]=6;
	//MenuParent[9]=2;
	//MenuChildFirst[9]=0;
	//MenuChildLast[9]=0;
	//Clock child 10-12(dntactive)
	MenuName[10]="Time";
	MenuType[10]=5;
	MenuParent[10]=3;
	MenuChildFirst[10]=0;
	MenuChildLast[10]=0;

	MenuName[11]="Date";
	MenuType[11]=6;
	MenuParent[11]=3;
	MenuChildFirst[11]=0;
	MenuChildLast[11]=0;

	MenuName[12]="Clock Face";
	MenuType[12]=1;
	MenuParent[12]=3;
	MenuChildFirst[12]=0;
	MenuChildLast[12]=0;
	//Screen child 13-14
	MenuName[13]="Brightness";
	MenuType[13]=4;
	MenuParent[13]=4;
	MenuChildFirst[13]=0;
	MenuChildLast[13]=0;

	MenuName[14]="Backlight";
	MenuType[14]=3;
	MenuParent[14]=4;
	MenuChildFirst[14]=0;
	MenuChildLast[14]=0;

	//MenuName[15]="Invert";
	//MenuType[15]=8;
	//MenuParent[15]=4;
	//MenuChildFirst[15]=0;
	//MenuChildLast[15]=0;
	//Bluetooth child 15-17
	MenuName[16]="Status";
	MenuType[16]=8;
	MenuParent[16]=5;
	MenuChildFirst[16]=0;
	MenuChildLast[16]=0;

	MenuName[17]="Sync";
	MenuType[17]=8;//?
	MenuParent[17]=5;
	MenuChildFirst[17]=0;
	MenuChildLast[17]=0;

	MenuName[18]="Information";
	MenuType[18]=1;
	MenuParent[18]=5;
	MenuChildFirst[18]=0;
	MenuChildLast[18]=0;
	//Device child 18-20
	MenuName[19]="Battery";
	MenuType[19]=2;
	MenuParent[19]=6;
	MenuChildFirst[19]=0;
	MenuChildLast[19]=0;

	MenuName[20]="Reboot";
	MenuType[20]=2;
	MenuParent[20]=6;
	MenuChildFirst[20]=0;
	MenuChildLast[20]=0;

	MenuName[21]="Information";
	MenuType[21]=2;
	MenuParent[21]=6;
	MenuChildFirst[21]=0;
	MenuChildLast[21]=0;
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

/*Циферблат*/
void DigitalClockFace(){
	//Правая колонка
	if(time(2)!=dayFixed||!printDates){
		printDates=false;
		lcd.clearScreen();
		lcd.setTextSize(1);
		//День недели
		lcd.setCursor(66,35);
		lcd.print(namesDays[time(3)-1]);
		//День месяца
		lcd.setCursor(66,49);
		lcd.print(time(2));
		//Месяц
		lcd.setCursor(66,63);
		lcd.print(namesMonths[time(1)-1]);
		//Прочее
		lcd.setCursor(66,78);
		//lcd.print(analogRead(0));
		if(alarmStatus) lcd.print("@");
		/*if(voltage>=4.0){
			lcd.setTextColor(GREEN);
			lcd.print("{");
			lcd.setTextColor(WHITE);
		}
		else if(voltage<4.0&&voltage>3.0){
			lcd.setTextColor(YELLOW);
			lcd.print("}");
			lcd.setTextColor(WHITE);
		}
		else{
			lcd.setTextColor(RED);
			lcd.print("`");
			lcd.setTextColor(WHITE);
		}*/
		//Обновляем переменную
		dayFixed=time(2);
	}
	//Часы
	if(time(4)!=hourFixed||!printDates){
		lcd.setTextSize(2);
		lcd.setTextColor(BLACK);
		lcd.setCursor(30,32);
		if(hourFixed<10&&time(4)!=0){//time(4)!=для 23, 59 и прочих
			if(time(4)<10) lcd.setCursor(48,32);//fixed
			else lcd.print("0");
			lcd.print(hourFixed);
		}
		else{
			lcd.print(hourFixed);
		}
		hourFixed=time(4);
		lcd.setTextColor(WHITE);
		lcd.setCursor(30,32);
		if(time(4)<10) lcd.print("0");
		lcd.print(time(4));
	}
	//Минуты
	if(time(5)!=minuteFixed||!printDates){
		lcd.setTextSize(2);
		lcd.setTextColor(BLACK);
		lcd.setCursor(30,62);
		if(minuteFixed<10&&time(5)!=0){
			if(time(5)<10) lcd.setCursor(48,62);
			else lcd.print("0");
			lcd.print(minuteFixed);
		}
		/*else if(time(4)==2&&time(5)==28){
			lcd.setTextColor(BLACK);
			lcd.setCursor(30,32);
			lcd.print("0");
			lcd.setCursor(30,62);
		}*/
		else{
			lcd.print(minuteFixed);
		}
		minuteFixed=time(5);
		lcd.setTextColor(WHITE);
		lcd.setCursor(30,62);
		if(time(5)<10) lcd.print("0");
		lcd.print(time(5));
		if(!printDates) printDates=true;
	}
}

/*Функция определения статуса кнопки*/
boolean pressed(byte button){//Возращает true если кнопка нажата
	if(analogRead(button)<=10) return true;
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

/*Отрисовка меню*/
void drawMenu(){
	if(!renderingStatics){
		lcd.setTextColor(WHITE);
		lcd.setTextSize(1);
		byte menuCursorPos=1;//Позиция курсора в принте меню
		for(byte i=MenuChildFirst[MenuLevel]; i<MenuChildLast[MenuLevel]+1;i++){
			lcd.setCursor(3,menuCursorPos);
			menuCursorPos=menuCursorPos+16;//Если надумаю ставить название меню просто поменять принт и сложение местами
			lcd.print(MenuName[i]);
		}
		renderingStatics=true;//Рендер выполнен, что бы не отрисовывать снова переменную переводим в true
	}
	lcd.drawFastVLine(0,MenuCurPos*16+3,14,WHITE);
}

void lightTimeSettings(){
	if(!renderingStatics){
		lcd.setTextColor(WHITE);
		lcd.setTextSize(1);
		//Содержимое
		lcd.setCursor(32,2);
		lcd.print("Backlight");
		lcd.setCursor(3,18);
		lcd.print("1min");
		lcd.setCursor(3,34);
		lcd.print("5min");
		lcd.setCursor(3,50);
		lcd.print("10min");
		lcd.setCursor(3,66);
		lcd.print("30min");
		lcd.setCursor(3,82);
		lcd.print("off");
		//Исходя из данных EEPROM устанавливаем курсор
		switch(backlightTimer){
			case 6:MenuCurPos=1;break;//1min
			case 30:MenuCurPos=2;break;//5min
			case 60:MenuCurPos=3;break;//10min
			case 180:MenuCurPos=4;break;//30min
			default:MenuCurPos=5;break;//off
		}
		renderingStatics=true;//Рендер выполнен
	}
	lcd.drawFastVLine(0,MenuCurPos*16+4,14,WHITE);
}

/*Настройка яркости*/
void brightSettings(){
	if(!renderingStatics){
		lcd.setTextSize(1);
		lcd.setTextColor(WHITE);
		lcd.setCursor(27,3);
		lcd.print("Brightness");

		staticData[0]=EEPROM.read(brightnessAddress);
		currentPer=staticData[0]*100/240;
		staticData[1]=currentPer;

		if(currentPer==100) lcd.setCursor(50,45);
		else if(currentPer<10) lcd.setCursor(59,45);
		else lcd.setCursor(55,45);
		lcd.print(currentPer);
		lcd.fillRect(4, 69, 1.2*currentPer, 10, WHITE);
		renderingStatics=true;
	}
	if(staticData[1]!=staticData[0]*100/240){
		currentPer=staticData[0]*100/240;
		lcd.setTextColor(BLACK);
		if(staticData[1]==100) lcd.setCursor(50,45);
		else if(staticData[1]<10) lcd.setCursor(59,45);
		else lcd.setCursor(55,45);
		lcd.print(staticData[1]);
		if(currentPer<staticData[1]) lcd.fillRect(1.2*currentPer+6, 69, 6, 10, BLACK);
		else if(currentPer>staticData[1]) lcd.fillRect(1.2*currentPer-2, 69, 6, 10, WHITE);
		staticData[1]=currentPer;
		//
		lcd.setTextColor(WHITE);
		if(currentPer==100) lcd.setCursor(50,45);
		else if(currentPer<10) lcd.setCursor(59,45);
		else lcd.setCursor(55,45);
		lcd.print(currentPer);
	}
}


/*Настройка времени, перед функцией использовать удаление старых символов*/
void timeSettings(byte hh, byte mm, byte cursor, char* menuName){
//byte час, byte mm, byte cursor (костыль, нужна функция подсчёта), boolean изменение текущего состояния (перересовывает)
	if(!renderingStatics){
		lcd.setTextSize(1);
		lcd.setTextColor(WHITE);
		lcd.setCursor(cursor,3);
		lcd.print(menuName);
		staticData[0]=hh;
		staticData[1]=mm;
		//Принт
		lcd.setCursor(44,55);
		if(staticData[0]<10) lcd.print("0");
		lcd.print(staticData[0]);
		lcd.print(":");
		if(staticData[1]<10) lcd.print("0");
		lcd.print(staticData[1]);
		renderingStatics=true;
	}
	switch(settingStep){
		case 0:
			lcd.setTextColor(WHITE);
			lcd.setCursor(44,55);
			if(staticData[0]<10) lcd.print("0");
			lcd.print(staticData[0]);
			//
			lcd.drawFastHLine(44, 72, 18, WHITE);
		break;
		case 1:
			lcd.setTextColor(WHITE);
			lcd.setCursor(66,55);
			if(staticData[1]<10) lcd.print("0");
			lcd.print(staticData[1]);
			//
			lcd.drawFastHLine(66, 72, 18, WHITE);
		break;
	}
}

void test(){
	if(Serial.available()){
		Serial.print("test");
	}
}

/*Настройка даты*/
void dateSettings(){
	if(!renderingStatics){
		lcd.setTextSize(1);
		lcd.setTextColor(WHITE);
		lcd.setCursor(47,3);
		lcd.print("Date");
		//
		staticData[0]=time(3);
		staticData[1]=time(2);
		staticData[2]=time(1);
		//
		lcd.setCursor(16,56);//День недели
		lcd.print(namesDays[staticData[0]-1]);

		lcd.setCursor(55,56);//Дата
		if(staticData[1]<10) lcd.print("0");

		lcd.print(staticData[1]);
		lcd.setCursor(81,56);//Месяц
		lcd.print(namesMonths[staticData[2]-1]);
		renderingStatics=true;
	}
	switch(settingStep){
		case 0:
			lcd.setTextColor(WHITE);
			lcd.setCursor(16,56);//День недели
			lcd.print(namesDays[staticData[0]-1]);
			lcd.drawFastHLine(15, 74, 35, WHITE);
			break;
		case 1:
			lcd.setTextColor(WHITE);
			lcd.setCursor(55,56);//Дата
			if(staticData[1]<10) lcd.print("0");
			lcd.print(staticData[1]);
			lcd.drawFastHLine(54, 74, 20, WHITE);
			break;
		case 2:
			lcd.setTextColor(WHITE);
			lcd.setCursor(81,56);//Месяц
			lcd.print(namesMonths[staticData[2]-1]);
			lcd.drawFastHLine(80, 74, 30, WHITE);
			break;
	}
}

/*Настройка статуса*/
boolean statusSettings(boolean currentStatus, boolean change = false){
	lcd.setCursor(55,1);//+8px padding
	if(change){
		lcd.setTextColor(BLACK);
		if(currentStatus) lcd.print("on");
		else lcd.print("off");
		currentStatus=!currentStatus;
		lcd.setCursor(55,1);
		lcd.setTextColor(WHITE);
	}
	if(currentStatus) lcd.print("on");
	else lcd.print("off");
	if(change) return currentStatus;
}

/*Включение/выключение блютуза*/
boolean bluetoothStatusChange(boolean status){
	if(status){
		Serial.begin(9600);
		//analogWrite(bluetoothPower, 255);
	}
	else{
		//analogWrite(bluetoothPower, 0);//Выключение bt
		Serial.end();//закрытие Serial
	}
	bluetoothStatus=status;
	return status;
}

/*Будильник*/
void AlarmClock(){
	if(time(4)==alarmHour&&time(5)==alarmMinute&&alarmStatus&&MenuType[MenuLevel]!=7){
		if(vibrationCycle==0&&renderingStatics==true){
			renderingStatics=false;
			vibrationCycle=1;
		}
		if(!renderingStatics){
			lcd.clearScreen();
			lcd.setTextSize(1);
			lcd.setTextColor(WHITE);
			lcd.setCursor(44,27);
			lcd.print("Alarm");
			//
			lcd.setCursor(44,55);
			if(alarmHour<10) lcd.print("0");
			lcd.print(alarmHour);
			lcd.print(":");
			if(alarmMinute<10) lcd.print("0");
			lcd.print(alarmMinute);
			serviceWork=true;
			renderingStatics=true;
		}
		if(pressed(ok)&&millisDelay(1)||pressed(up)&&millisDelay(1)||pressed(down)&&millisDelay(1)){//&&millisDelay(1)
			analogWrite(vibration, 0);
			vibrationCycle=0;
			alarmStatus=false;
			serviceWork=false;
			printDates=false;//!!
			renderingStatics=false;
			lcd.clearScreen();
		}
		if(pressed(back)&&millisDelay(1)){
			analogWrite(vibration, 0);
			alarmMinute+=5;
			vibrationCycle=0;//Обнуляем ибо словим проблему в будущем
			alarmStatus=true;//Будильник установлен снова
			serviceWork=false;//Сервис прерывающий прочие программы отключен
			printDates=false;//Даём комманды на то что даты не отрисованы, нужно перерисовать
			renderingStatics=false;//Отрисовка статики не выполнена
			lcd.clearScreen();
		}
		if(millisDelay(10)){
			vibrationCycle++;
			if(vibrationCycle>7) vibrationCycle=1;
			analogWrite(vibration, vibrationMode[vibrationCycle-1]);
		}
	}
	else if(time(4)==alarmHour&&time(5)==alarmMinute+1&&alarmStatus&&serviceWork){
		analogWrite(vibration, 0);
		alarmMinute+=5;
		vibrationCycle=0;//Обнуляем ибо словим проблему в будущем
		alarmStatus=true;//Будильник установлен снова
		serviceWork=false;//Сервис прерывающий прочие программы отключен
		printDates=false;//Даём комманды на то что даты не отрисованы, нужно перерисовать
		renderingStatics=false;//Отрисовка статики не выполнена
		lcd.clearScreen();
	}
}

int btBuffer;
int btData[2] = {0, 0};//2, 4, 8, 16, ...
byte btDataCycle = 0;
byte btDataLen = 0;

void bluetooth(){
	if(Serial.available()){
		btBuffer=Serial.read();
		switch(char(btBuffer)){
			case '.'://46
				btDataCycle++;
				btDataLen=0;
				break;
			case ';'://
				btBuffer=0;
				btDataCycle=0;
				btDataLen=0;
				Serial.println("SETTING TRANSFERRED\n\r");
				break;
			default:
				if(btDataLen==0) btData[btDataCycle]=char(btBuffer);
				else btData[btDataCycle]=btData[btDataCycle]*10+char(btBuffer);
				btDataLen++;
				break;
		}
	}
}

void btHandler(){

}

/*Функция подсчёта питающего напряжения*/
float readVcc(){
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	delayMicroseconds(300);//Пауза раз в 30 секунд
	ADCSRA |= _BV(ADSC); // начало преобразований
	while (bit_is_set(ADCSRA, ADSC)); // измерение
	uint8_t low = ADCL; // сначала нужно прочесть ADCL - это запирает ADCH
	uint8_t high = ADCH; // разлочить оба
	float result = (high<<8) | low;
	result = (1.1 * 1023.0) / result; // Результат Vcc в милливольтах
	voltage = result-0.03;//Переносим результат в глобальную переменную voltage
}

/*Функция рестарта*/
void(* resetFunc) (void) = 0;

void setup(){
	/*Установка настроек DS1302*/
	rtc.writeProtect(false);//Защита от записи
	rtc.halt(false);
	//Год, Месяц, день, час, минуты, секунда, день недели
	Time t(2016, 02, 11, 23, 59, 50, 6);//Загоняем в функцию
	//rtc.time(t);//Устанавливаем время
	/*Установка пинов*/
	//Аналоговые
	pinMode(A0, INPUT);
	pinMode(A1, INPUT);
	pinMode(A2, INPUT);
	pinMode(A3, INPUT);
	pinMode(A4, INPUT);
	//Цифровые
	//pinMode(bluetoothPower, OUTPUT);//Блютуз вкл/выкл
	pinMode(backlight, OUTPUT);
	//pinMode(vibration, OUTPUT);//вибромотор вкл/выкл
	//Прочее
	analogReference(DEFAULT);
	/*Чтение EEPROM*/
	//Яркость
	brightness=EEPROM.read(brightnessAddress);
	backlightTimer=EEPROM.read(backlightTimerAddress);//(6 хранится 120 на выходе)*10*2
	//Будильник
	alarmMinute=EEPROM.read(ACMinute);
	alarmHour=EEPROM.read(ACHour);
	/*Установка переменных*/
	//Фиксированные даты
	dayFixed=time(2);
	hourFixed=time(4);
	minuteFixed=time(5);
	//Таймеры millis
	currentTime = millis()/100;
	loopTime = currentTime;
	voltageTime = currentTime;
	//Прочие перменные
	alarmStatus=false;//Будильник выкл
	/*Выполнение функций*/
	MenuSetup();//Загружаем меню
	bluetoothStatusChange(false);//Отвключаем блютуз
	delay(1);
	/*Инциализация дисплея*/
	lcd.begin();
	//lcd.setRotation(2);
	lcd.invertDisplay(false);
	delay(1);
	//Установка яркости
	analogWrite(backlight, brightness);
	//Serial.begin(9600);
}

void loop(){
	/*Обновляем текущее время*/
	currentTime = millis()/100;
	//test();
	//Serial.print("!!!");
	/*Проверяем будильник (сервисные комманды)*/
	AlarmClock();
	/*Если есть сервисная работа - отменяем выполнение программы*/
	if(serviceWork==true) return;
	//bluetooth();
	/*Установка таймеров кнопки*/
	switch(MenuType[MenuLevel]){
		case 4: TimerButton = 3; break;
		case 5: TimerButton = 3; break;
		case 6: TimerButton = 3; break;
		case 7: TimerButton = 3; break;
		default: TimerButton = 5; break;
	}
	/*Действия кнопок*/
	/*ОК*/
	if(pressed(ok)&&millisDelay(TimerButton)&&MenuType[MenuLevel]!=2){
		renderingStatics=false;
		switch(MenuType[MenuLevel]){
			case 3://Время подсветки
				switch(MenuCurPos){
					case 1: backlightTimer=6; break;
					case 2: backlightTimer=30; break;
					case 3: backlightTimer=60; break;
					case 4: backlightTimer=180; break;
					case 5: backlightTimer=0; break;
				}
				EEPROM.write(backlightTimerAddress, backlightTimer);
				MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
				MenuLevel=MenuParent[MenuLevel];
				lcd.clearScreen();
				break;
			case 4://Яркость
				EEPROM.write(brightnessAddress, staticData[0]);
				MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
				MenuLevel=MenuParent[MenuLevel];
				lcd.clearScreen();
				break;
			case 5://Время
				settingStep++;
				if(settingStep>1){
					Time t(2016, time(1), time(2), staticData[0], staticData[1], 0, time(3));
					rtc.time(t);//Устанавливаем время
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
					settingStep=0;
					lcd.clearScreen();
				}
				else renderingStatics=true;//Что бы не выполялась функция timeSettings() в блоке с типом меню
				lcd.drawFastHLine(44, 72, 44, BLACK);
				break;
			case 6:
				settingStep++;
				lcd.drawFastHLine(10, 74, 108, BLACK);
				if(settingStep>2){
					Time t(2016, staticData[2], staticData[1], time(4), time(5), time(6),staticData[0]);
					rtc.time(t);//Устанавливаем время
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
					settingStep=0;
					lcd.clearScreen();
				}
				else renderingStatics=true;
				break;
			case 7://Время будильника
				settingStep++;
				if(settingStep>1){
					EEPROM.write(ACHour, staticData[0]);
					EEPROM.write(ACMinute, staticData[1]);
					alarmHour=staticData[0];
					alarmMinute=staticData[1];
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
					settingStep=0;
					lcd.clearScreen();
				}
				else renderingStatics=true;//Что бы не выполялась функция timeSettings() в блоке с типом меню
				lcd.drawFastHLine(44, 72, 44, BLACK);
				break;
			default:
				if(MenuType[MenuChildFirst[MenuLevel]+MenuCurPos]==8){//Настройка ВКЛ/ВЫКЛ
					switch(MenuLevel){
						case 2: alarmStatus=statusSettings(alarmStatus, true); break;
						//case 4: statusSettings(alarmStatus, true); break;
						case 5: bluetoothStatus=statusSettings(bluetoothStatusChange(bluetoothStatus), true); break;
					}
					return;
				}
				//
				MenuLevel=MenuChildFirst[MenuLevel]+MenuCurPos;
				MenuCurPos=0;
				lcd.clearScreen();
				break;
		}
	}

	/*Назад*/
	if(pressed(back)&&millisDelay(TimerButton)&&MenuLevel!=0){
		renderingStatics=false;
		switch(MenuType[MenuLevel]){
			case 4://Яркость
				analogWrite(backlight, brightness);
				MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
				MenuLevel=MenuParent[MenuLevel];
				lcd.clearScreen();
				break;
			case 5://Время
				settingStep--;
				if(settingStep==255){
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
					settingStep=0;
					lcd.clearScreen();
				}
				else renderingStatics=true;
				lcd.drawFastHLine(44, 72, 44, BLACK);
				break;
			case 6:
				settingStep--;
				lcd.drawFastHLine(10, 74, 108, BLACK);
				if(settingStep==255){
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
					settingStep=0;
					lcd.clearScreen();
				}
				else renderingStatics=true;
				break;
			case 7://Время будильника
				settingStep--;
				if(settingStep==255){
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
					settingStep=0;
					lcd.clearScreen();
				}
				else renderingStatics=true;
				lcd.drawFastHLine(44, 72, 44, BLACK);
				break;
			default:
				MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
				MenuLevel=MenuParent[MenuLevel];
				if(MenuLevel==0) printDates=false;
				lcd.clearScreen();
				break;
		}
	}

	/*Вверх*/
	if(pressed(up)&&millisDelay(TimerButton)){
		switch(MenuType[MenuLevel]){
			case 1://Меню
				MenuCurPos--;
				MenuCurPos=limValue(MenuCurPos, MenuChildLast[MenuLevel]-MenuChildFirst[MenuLevel]);
				lcd.drawFastVLine(0, 0, 128, BLACK);//Рисуем линию, убираем белый курсор
				break;
			case 3://Время подсветки
				MenuCurPos--;
				MenuCurPos = limValue(MenuCurPos, 5, 1);
				lcd.drawFastVLine(0, 0, 128, BLACK);
				break;
			case 4://Яркость
				staticData[0]+=12;
				if(staticData[0]>240) staticData[0]=240;
				analogWrite(backlight, staticData[0]);
				brightSettings();
				break;
			case 5://Время
				switch(settingStep){
					case 0:
						lcd.setTextColor(BLACK);
						lcd.setCursor(44,55);
						if(staticData[0]<10) lcd.print("0");
						lcd.print(staticData[0]);
						//
						staticData[0]++;
						staticData[0] = limValue(staticData[0],23);
						timeSettings(staticData[0], staticData[1], 46, "Time");
						break;
					case 1:
						lcd.setTextColor(BLACK);
						lcd.setCursor(66,55);
						if(staticData[1]<10) lcd.print("0");
						lcd.print(staticData[1]);
						//
						staticData[1]++;
						staticData[1] = limValue(staticData[1],59);
						timeSettings(staticData[0], staticData[1], 46, "Time");
						break;//44, "Alarm"
				}
				break;
			case 6:
				switch(settingStep){
					case 0:
						lcd.setTextColor(BLACK);
						lcd.setCursor(16,56);//День недели
						lcd.print(namesDays[staticData[0]-1]);
						staticData[0]++;
						staticData[0] = limValue(staticData[0], 7, 1);
						break;
					case 1:
						lcd.setTextColor(BLACK);
						lcd.setCursor(55,56);//День
						if(staticData[1]<10) lcd.print("0");
						lcd.print(staticData[1]);
						staticData[1]++;
						staticData[1] = limValue(staticData[1], daysinMonths[staticData[2]-1], 1);
						break;
					case 2:
						lcd.setTextColor(BLACK);
						lcd.setCursor(81,56);//Месяц
						lcd.print(namesMonths[staticData[2]-1]);
						staticData[2]++;
						staticData[2] = limValue(staticData[2], 12, 1);
						break;
				}
				break;
			case 7://Время будильника
				switch(settingStep){
					case 0:
						lcd.setTextColor(BLACK);
						lcd.setCursor(44,55);
						if(staticData[0]<10) lcd.print("0");
						lcd.print(staticData[0]);
						//
						staticData[0]++;
						staticData[0] = limValue(staticData[0],23);
						timeSettings(staticData[0], staticData[1], 44, "Alarm");
						break;
					case 1:
						lcd.setTextColor(BLACK);
						lcd.setCursor(66,55);
						if(staticData[1]<10) lcd.print("0");
						lcd.print(staticData[1]);
						//
						staticData[1]++;
						staticData[1] = limValue(staticData[1],59);
						timeSettings(staticData[0], staticData[1], 44, "Alarm");
						break;
				}
				break;
		}
	}

	/*Вниз*/
	if(pressed(down)&&millisDelay(TimerButton)){
		switch(MenuType[MenuLevel]){
			case 1://Меню
				MenuCurPos++;
				MenuCurPos=limValue(MenuCurPos, MenuChildLast[MenuLevel]-MenuChildFirst[MenuLevel]);
				lcd.drawFastVLine(0, 0, 128, BLACK);//Рисуем линию, убираем белый курсор
				break;
			case 3://Время подсветки
				MenuCurPos++;
				MenuCurPos = limValue(MenuCurPos, 5, 1);
				lcd.drawFastVLine(0, 0, 128, BLACK);
				break;
			case 4://Яркость
				staticData[0]-=12;
				if(staticData[0]<12) staticData[0]=12;
				analogWrite(backlight, staticData[0]);
				brightSettings();
				break;
			case 5://Время
				switch(settingStep){
					case 0:
						lcd.setTextColor(BLACK);
						lcd.setCursor(44,55);
						if(staticData[0]<10) lcd.print("0");
						lcd.print(staticData[0]);
						//
						staticData[0]--;
						staticData[0] = limValue(staticData[0],23);
						timeSettings(staticData[0], staticData[1], 46, "Time");
						break;
					case 1:
						lcd.setTextColor(BLACK);
						lcd.setCursor(66,55);
						if(staticData[1]<10) lcd.print("0");
						lcd.print(staticData[1]);
						//
						staticData[1]--;
						staticData[1] = limValue(staticData[1],59);
						timeSettings(staticData[0], staticData[1], 46, "Time");
						break;
				}
				break;
			case 6:
				switch(settingStep){
					case 0:
						lcd.setTextColor(BLACK);
						lcd.setCursor(16,56);//День недели
						lcd.print(namesDays[staticData[0]-1]);
						staticData[0]--;
						staticData[0] = limValue(staticData[0], 7, 1);
						break;
					case 1:
						lcd.setTextColor(BLACK);
						lcd.setCursor(55,56);//День
						if(staticData[1]<10) lcd.print("0");
						lcd.print(staticData[1]);
						staticData[1]--;
						staticData[1] = limValue(staticData[1], daysinMonths[staticData[2]-1], 1);
						break;
					case 2:
						lcd.setTextColor(BLACK);
						lcd.setCursor(81,56);//Месяц
						lcd.print(namesMonths[staticData[2]-1]);
						staticData[2]--;
						staticData[2] = limValue(staticData[2], 12, 1);
						break;
				}
				break;
			case 7://Время будильника
				switch(settingStep){
					case 0:
						lcd.setTextColor(BLACK);
						lcd.setCursor(44,55);
						if(staticData[0]<10) lcd.print("0");
						lcd.print(staticData[0]);
						//
						staticData[0]--;
						staticData[0] = limValue(staticData[0],23);
						timeSettings(staticData[0], staticData[1], 44, "Alarm");
						break;
					case 1:
						lcd.setTextColor(BLACK);
						lcd.setCursor(66,55);
						if(staticData[1]<10) lcd.print("0");
						lcd.print(staticData[1]);
						//
						staticData[1]--;
						staticData[1] = limValue(staticData[1],59);
						timeSettings(staticData[0], staticData[1], 44, "Alarm");
						break;
				}
				break;
		}
	}

	/*Настройка статуса*/
	if(MenuType[MenuChildFirst[MenuLevel]]==8&&!renderingStatics){
		switch(MenuLevel){
			case 2: statusSettings(alarmStatus); break;
			//case 4: statusSettings(false); break;
			case 5: statusSettings(bluetoothStatus); break;
		}
	}

	/*Использование функций по типу меню*/
	switch(MenuType[MenuLevel]){
		case 0: DigitalClockFace(); break;
		case 1: drawMenu(); break;
		case 3: lightTimeSettings(); break;
		case 4: brightSettings(); break;
		case 5: timeSettings(time(4), time(5), 46, "Time"); break;
		case 6: dateSettings(); break;
		case 7: timeSettings(alarmHour, alarmMinute, 44, "Alarm"); break;
	}

	/*Использование функций по MenuLevel*/
	switch(MenuLevel){
		case 19:
			if(!renderingStatics){
				//left top
				lcd.fillRect(14,20,23,4,WHITE);
				lcd.fillRect(25,24,46,80,WHITE);
				renderingStatics=true;
			}
			break;
		case 20://Перезагрузка
			analogWrite(backlight, 0);
			resetFunc();
			break;
		case 21://Информация
			if(!renderingStatics){
				lcd.setTextSize(1);
				lcd.setCursor(26,0);
				lcd.print("Information");
				lcd.setCursor(2,16);
				lcd.print("ROM version");
				lcd.setCursor(2,32);
				lcd.print("0.9 beta");
				lcd.setCursor(2,48);
				lcd.print("SOC");
				lcd.setCursor(2,64);
				lcd.print("ATmega328p");
				lcd.setCursor(2,80);
				lcd.print("Free RAM");
				lcd.setCursor(2,96);
				lcd.print(freeMemory());
				lcd.print("/2048kb");
				renderingStatics=true;
			}
			break;
	}
}
