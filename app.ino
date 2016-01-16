/*
weOS ROM 0.8.1
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
</cl>
*/
#include <SPI.h>
//#include <MsTimer2.h>
#include <EEPROM.h>
#include <MemoryFree.h>
#include <TFT_ILI9163C.h>
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
//Прочее (digital)
#define bluetoothPower 3
//Прочее (digital ШИМ)
#define vibration 5//3old
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
---|Other pin's
vibration 5
backlight (__LED) 9
*/
#define __CS 10
#define __DC 6
#define __RST 8

/*Установка пинов для экрана*/
TFT_ILI9163C lcd = TFT_ILI9163C(__CS, __DC, __RST);

/*Переменные*/
/*Для работы с таймерами (?long?)*/
long currentTime;
long loopTime;
long voltageTime;
byte TimerButton;

double voltage;

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

/*Для отображения дат, месяца в буквах*/
const char* namesDays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char* namesMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const byte daysinMonths[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

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
//Экран
byte brightness;
byte backlightTimer;

/*Для работы с настройками*/
byte settingStep;
byte currentPer;
int fixedSetNum;//Фиксированная цифра настроек (для удаление прошлой)

/*Прочее*/
boolean devMode = false;//Деволперский режим
boolean printDates = false;//Спец.переменная для работы с датами
boolean sleepMode = false;//Режим сна
boolean renderingStatics = false;//Отрисовка статических изображений
boolean serviceWork = false;//Работа какого то сервиса который может выполнятся в момент работы другой программы, например будильник

/*Переменные для работы меню/c меню*/
//Основа построения меню была взята с geektimes.ru
//http://geektimes.ru/post/255408/
char* MenuName[21];//Имя меню
//MenuType = 0;// - циферблат
//MenuType = 1;// - стандартное меню с дочерними элементами
//MenuType = 2;// - элементы БЕЗ дочерних элементов
byte MenuType[21];//Тип меню
byte MenuParent[21];//Родитель меню
byte MenuChildFirst[21];//Первый потомок
byte MenuChildLast[21];//Последний потомок
/**Переменные для работы навигации в меню**/
unsigned int MenuLevel = 0;//Уровень меню (от ok, back)
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
	MenuChildLast[2]=9;

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
	MenuChildFirst[5]=15;
	MenuChildLast[5]=17;

	MenuName[6]="Device";
	MenuType[6]=1;
	MenuParent[6]=1;
	MenuChildFirst[6]=18;
	MenuChildLast[6]=20;
	//Alarm child 7-9
	MenuName[7]="Status";
	MenuType[7]=8;
	MenuParent[7]=2;
	MenuChildFirst[7]=0;
	MenuChildLast[7]=0;

	MenuName[8]="Time";
	MenuType[8]=5;
	MenuParent[8]=2;
	MenuChildFirst[8]=0;
	MenuChildLast[8]=0;

	MenuName[9]="Date";
	MenuType[9]=6;
	MenuParent[9]=2;
	MenuChildFirst[9]=0;
	MenuChildLast[9]=0;
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
	//Bluetooth child 15-17
	MenuName[15]="Status";
	MenuType[15]=8;
	MenuParent[15]=5;
	MenuChildFirst[15]=0;
	MenuChildLast[15]=0;

	MenuName[16]="Sync";
	MenuType[16]=8;//?
	MenuParent[16]=5;
	MenuChildFirst[16]=0;
	MenuChildLast[16]=0;

	MenuName[17]="Information";
	MenuType[17]=1;
	MenuParent[17]=5;
	MenuChildFirst[17]=0;
	MenuChildLast[17]=0;
	//Device child 18-20
	MenuName[18]="Battery";
	MenuType[18]=2;
	MenuParent[18]=6;
	MenuChildFirst[18]=0;
	MenuChildLast[18]=0;

	MenuName[19]="Reboot";
	MenuType[19]=2;
	MenuParent[19]=6;
	MenuChildFirst[19]=0;
	MenuChildLast[19]=0;

	MenuName[20]="Information";
	MenuType[20]=2;
	MenuParent[20]=6;
	MenuChildFirst[20]=0;
	MenuChildLast[20]=0;
}

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
	//Пермещено ко всему остальному, но работа та же
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
	//year = 0 months = 1 days = 2 numWeekDay = 3 hours = 4 minutes = 5 seconds = 6
	switch(arg){
		case 0://Год
			return year;
		case 1://Месяц
			return month;
		case 2://Дата
			return day;
		case 3://День недели
			return numWeekDay;
		case 4://Часы
			return hours;
		case 5://Минуты
			return minutes;
		case 6://Секунды
			return seconds;
		case 255://Ни черта
			break;
	}
}

/*Циферблат*/
void DigitalClockFace(){
	//Правая колонка
	if(time(2)!=dayFixed||!printDates){
		printDates=false;
		lcd.clearScreen();
		lcd.setTextSize(1);
		//lcd.setTextColor(WHITE);
		//День недели
		lcd.setCursor(66,35);
		lcd.print(namesDays[time(3)-1]);
		//День месяца
		lcd.setCursor(66,49);
		lcd.print(time(2));
		//Месяц
		lcd.setCursor(66,63);
		lcd.print(namesMonths[time(1)-1]);
		//
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
	//if(debugging) Serial.println(analogRead(button));
}

/*Функция пауз для кнопок*/
boolean millisDelay(byte delay){//Возращает true если таймер нажатия кнопки прошёл
	if(currentTime>=loopTime+delay){
		loopTime=currentTime;
		return true;
	}
	else return false;
}

/*Отрисовка меню*/
void DrawMenu(){
	lcd.setTextSize(1);
	if(!renderingStatics){
		byte menuCursorPos=1;//Позиция курсора в принте меню
		for(int i=MenuChildFirst[MenuLevel];i<MenuChildLast[MenuLevel]+1;i++){
			lcd.setCursor(3,menuCursorPos);
			menuCursorPos=menuCursorPos+16;//Если надумаю ставить название меню просто поменять принт и сложение местами
			lcd.print(MenuName[i]);
		}
		renderingStatics=true;
	}
	lcd.drawFastVLine(0,MenuCurPos*16+3,14,WHITE);
}

/*Настройки таймаута экрана, пунктирная настройка*/
void ListSettings(){
	lcd.setTextSize(1);//10+6
	if(!renderingStatics){
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
		switch(backlightTimer){//устанавливаем курсор
			case 6://1min
				MenuCurPos=1;
				break;
			case 30://5min
				MenuCurPos=2;
				break;
			case 60://10min
				MenuCurPos=3;
				break;
			case 180://30min
				MenuCurPos=4;
				break;
			default://0
				MenuCurPos=5;
				break;
		}
		renderingStatics=true;
	}
	if(MenuCurPos==0) MenuCurPos=1;
	lcd.drawFastVLine(0,MenuCurPos*16+4,14,WHITE);
}

/*Настройка яркости, полосочная настройка*/
void BandSettings(){
	currentPer=brightness*100/240;
	if(!renderingStatics){
		lcd.setTextSize(1);
		lcd.setCursor(27,3);
		lcd.print("Brightness");
		lcd.fillRect(4, 69, 1.2*currentPer, 10, WHITE);
		renderingStatics=true;
	}
	if(currentPer!=fixedSetNum){
		lcd.setTextColor(BLACK);
		if(fixedSetNum==100){
			lcd.setCursor(50,45);
		}
		else if(fixedSetNum<10){
			lcd.setCursor(59,45);
		}
		else{
			lcd.setCursor(55,45);
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
	if(fixedSetNum==100){
		lcd.setCursor(50,45);
	}
	else if(fixedSetNum<10){
		lcd.setCursor(59,45);
	}
	else{
		lcd.setCursor(55,45);
	}
	lcd.print(currentPer);
}

/*Настройка времени*/
void TimeSettings(){
	lcd.setTextSize(1);
	lcd.setTextColor(WHITE);
	if(!renderingStatics){
		lcd.setCursor(46,3);
		lcd.print("Time");
		renderingStatics=true;
	}
	//main
	lcd.setCursor(44,55);
	if(time(4)<10) lcd.print("0");
	lcd.print(time(4));
	lcd.print(":");
	if(time(5)<10) lcd.print("0");
	lcd.print(time(5));
	switch(settingStep){
		case 0:
			lcd.drawFastHLine(44, 72, 18, WHITE);
			break;
		case 1:
			lcd.drawFastHLine(66, 72, 18, WHITE);
			break;
	}
}

/*Настройки даты*/
void DateSettings(){
	lcd.setTextSize(1);
	lcd.setTextColor(WHITE);
	if(!renderingStatics){
		lcd.setCursor(47,3);
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

/*Настройки будильника*/
void AlarmSettings(){
	lcd.setTextSize(1);
	lcd.setTextColor(WHITE);
	if(renderingStatics==false){
		lcd.setCursor(44,3);
		lcd.print("Alarm");
		renderingStatics=true;
	}
	//main
	lcd.setCursor(44,55);
	if(alarmHour<10) lcd.print("0");
	lcd.print(alarmHour);
	lcd.print(":");
	if(alarmMinute<10) lcd.print("0");
	lcd.print(alarmMinute);
	switch(settingStep){
		case 0:
			lcd.drawFastHLine(44, 72, 18, WHITE);
			break;
		case 1:
			lcd.drawFastHLine(66, 72, 18, WHITE);
			break;
	}
}

/*Будильник*/
void AlarmClock(){
	if(time(4)==alarmHour&&time(5)==alarmMinute&&alarmStatus&&MenuType[MenuLevel]!=7){
		if(vibrationCycle==0){
			lcd.clearScreen();
			lcd.setTextSize(1);
			lcd.setCursor(44,27);
			lcd.print("Alarm");
			//рисуем время
			lcd.setTextSize(1);
			lcd.setCursor(44,55);
			if(alarmHour<10) lcd.print("0");
			lcd.print(alarmHour);
			lcd.print(":");
			if(alarmMinute<10) lcd.print("0");
			lcd.print(alarmMinute);
			serviceWork=true;//Сервис работает
		}
		if(pressed(ok)&&millisDelay(1)){//Если нажата кнопка ok во время звонка будильника
			lcd.clearScreen();
			analogWrite(5,0);
			vibrationCycle=0;
			alarmStatus=false;
			serviceWork=false;
			printDates=false;
			renderingStatics=false;
		}
		if(pressed(back)&&millisDelay(1)){//Если нажата кнопка ok во время звонка будильника
			lcd.clearScreen();//Очищаем дисплей
			analogWrite(5,0);//Выключаем вибромотор
			alarmMinute+=5;
			vibrationCycle=0;//Обнуляем ибо словим проблему в будущем
			alarmStatus=true;//Будильник установлен снова
			serviceWork=false;//Сервис прерывающий прочие программы отключен
			printDates=false;//Даём комманды на то что даты не отрисованы, нужно перерисовать
			renderingStatics=false;//Отрисовка статики не выполнена
		}
		if(millisDelay(10)){//Работа вибромотора, добавление 1 цикла по истечению таймера в 1сек. (bD - экономия места)
			vibrationCycle++;
			if(vibrationCycle>7) vibrationCycle=1;
			analogWrite(5, vibrationMode[vibrationCycle-1]);
		}
	}
	else if(time(4)==alarmHour&&time(5)==alarmMinute+1&&alarmStatus&&serviceWork){//если на 1 минуту раньше поставить будильник не ставит, юзаю serviceWork
		lcd.clearScreen();//Очищаем дисплей
		analogWrite(5,0);//Выключаем вибромотор
		alarmMinute+=5;
		vibrationCycle=0;//Обнуляем ибо словим проблему в будущем
		alarmStatus=true;//Будильник установлен снова
		serviceWork=false;//Сервис прерывающий прочие программы отключен
		printDates=false;//Даём комманды на то что даты не отрисованы, нужно перерисовать
		renderingStatics=false;//Отрисовка статики не выполнена
	}
}

/*Настройка статуса*/
boolean StatusSettings(boolean currentStatus, boolean change = false){
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

/*Функция подсчёта питающего напряжения*/
float readVcc(){
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	delayMicroseconds(300);
	ADCSRA |= _BV(ADSC); // начало преобразований
	while (bit_is_set(ADCSRA, ADSC)); // измерение
	uint8_t low = ADCL; // сначала нужно прочесть ADCL - это запирает ADCH
	uint8_t high = ADCH; // разлочить оба
	float result = (high<<8) | low;
	result = (1.1 * 1023.0) / result; // Результат Vcc в милливольтах
	voltage = result-0.03;//Переносим результат в глобальную переменную voltage
}

int btBuffer;
int btData[2] = {0, 0};//2, 4, 8, 16, ...
byte btDataCycle = 0;
byte btDataLen = 0;

/**/
void bluetooth(){
	if(Serial.available()){
		btBuffer=Serial.read();
		switch(btBuffer){
			case 35:// '#'
				Serial.print("minutes:1\n\rhours:2\n\rday:3\n\rwd:4\n\rmnt:5\n\r");
				break;
			case 37:// '%' - voltage
				Serial.print("\nVoltage:");
				Serial.println(voltage);
				break;
			case 33:
				Serial.println(hourFixed);
				Serial.print(":");
				Serial.println(minuteFixed);
				break;
			case 63:
				Serial.println(hours);
				Serial.print(":");
				Serial.println(minutes);
				break;
			case 46://'.'
				btDataCycle++;
				btDataLen=0;
				break;
			case 59://Если передан символ ';' означающий конец комманды
				btBuffer=0;
				btDataCycle=0;
				btDataLen=0;
				//btHandler=true;
				Serial.print("SETTING TRANSFERRED\n\r");
				Serial.println(btData[0]);
				Serial.print(".");
				Serial.println(btData[1]);
				btRequestHandler();
				break;
			default:
				if(btDataLen==0){
					btData[btDataCycle]=ASCII(btBuffer);
				}
				else{
					btData[btDataCycle]=btData[btDataCycle]*10+ASCII(btBuffer);
				}
				btDataLen++;
				break;
		}
	}
}

/*Обработчик блютуз-запросов*/
void btRequestHandler(){
	switch(btData[0]){
		case 1:
			EEPROM.write(minuteAddress,btData[1]);
			minutes=btData[1];
			btData[1]=0;
		break;
		case 2:
			EEPROM.write(hourAddress,btData[1]);
			hours=btData[1];
			btData[1]=0;
		break;
		case 3:
			EEPROM.write(dayAddress,btData[1]);
			day=btData[1];
			btData[1]=0;
			printDates=false;
			break;
		case 4:
			EEPROM.write(numWeekDayAddress,btData[1]);
			numWeekDay=btData[1];
			btData[1]=0;
			printDates=false;
			break;
		case 5:
			EEPROM.write(monthAddress,btData[1]);
			month=btData[1];
			btData[1]=0;
			printDates=false;
			break;
	}
}

/*Функция перевода цифровых символов из ASCII понятную систему*/
byte ASCII(byte arg){
	switch(arg){
		case 48:
			return 0;
		case 49:
			return 1;
		case 50:
			return 2;
		case 51:
			return 3;
		case 52:
			return 4;
		case 53:
			return 5;
		case 54:
			return 6;
		case 55:
			return 7;
		case 56:
			return 8;
		case 57:
			return 9;
		default:
			return arg;
	}
}

/*Функция рестарта*/
void(* resetFunc) (void) = 0;

void setup(){
	/*Таймер*/
	//MsTimer2::set(993, timerSeconds);
	//MsTimer2::start();
	//Аналоговые
	pinMode(A0, INPUT);
	pinMode(A1, INPUT);
	pinMode(A2, INPUT);
	pinMode(A3, INPUT);
	pinMode(A4, INPUT);
	//Цифровые
	pinMode(bluetoothPower, OUTPUT);
	pinMode(backlight, OUTPUT);
	pinMode(vibration, OUTPUT);
	/*Чтение EEPROM*/
	//Яркость
	brightness=EEPROM.read(brightnessAddress);
	backlightTimer=EEPROM.read(backlightTimerAddress);//(6 хранится 120 на выходе)*10*2
	//Будильник
	alarmMinute=EEPROM.read(ACMinute);
	alarmHour=EEPROM.read(ACHour);
	//Установка времени
	year=2016;
	month=EEPROM.read(monthAddress);//7[proverka]
	numWeekDay=EEPROM.read(numWeekDayAddress);
	day=EEPROM.read(dayAddress);
	hours=EEPROM.read(hourAddress);
	minutes=EEPROM.read(minuteAddress);
	seconds=50;
	/*Установка значений из памяти*/
	//Яркость
	analogReference(DEFAULT);
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
	alarmStatus=false;
	/*Выполнение функций*/
	MenuSetup();
	//readVcc();
	delay(1);
	/*Инциализация дисплея*/
	lcd.begin();
	analogWrite(backlight, brightness);
}

void loop(){
	//digitalWrite(bluetoothPower, HIGH);
	//powerSaveMode();
	//Обновляем текущее время
	currentTime = millis()/100;
	//Затрагиваем time(255) так как иногда в меню терялась дата
	AlarmClock();
	//Если сервис. функция работает, тогда прерываем работу программы.
	if(serviceWork==true) return;
	time(255);
	if(currentTime>=voltageTime+600){//раз в ~60секунд пересчитывать напряжение
		voltageTime=currentTime;
		//readVcc();
	}

	bluetooth();///////////////////////////TEST

	if(backlightTimer!=0&&currentTime>=loopTime+backlightTimer*100&&MenuLevel>0){
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
	//Таймер кнопки, где 10 - секунда
	switch(MenuType[MenuLevel]){
		case 4:
			TimerButton = 3;
			break;
		case 5:
			TimerButton = 3;
			break;
		case 6:
			TimerButton = 3;
			break;
		case 7:
			TimerButton = 3;
			break;
		default:
			TimerButton = 5;
			break;
	}
	//Обработка кнопок
	if(pressed(ok)&&millisDelay(TimerButton)&&MenuType[MenuLevel]!=2){
		renderingStatics=false;
		switch(MenuType[MenuLevel]){
			case 3:
				switch(MenuCurPos){
					case 1:
						backlightTimer=6;
						break;
					case 2:
						backlightTimer=30;
						break;
					case 3:
						backlightTimer=60;
						break;
					case 4:
						backlightTimer=180;
						break;
					case 5:
						backlightTimer=0;
						break;
				}
				EEPROM.write(backlightTimerAddress, backlightTimer);
				MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
				MenuLevel=MenuParent[MenuLevel];
				lcd.clearScreen();
				break;
			case 4://Яркость
				EEPROM.write(brightnessAddress, brightness);
				MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
				MenuLevel=MenuParent[MenuLevel];
				lcd.clearScreen();
				break;
			case 5://Время
				settingStep++;
				lcd.drawFastHLine(43, 72, 42, BLACK);
				if(settingStep>1){
					EEPROM.write(minuteAddress, minutes);
					EEPROM.write(hourAddress, hours);
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
					settingStep=0;
					lcd.clearScreen();
				}
				break;
			case 6://Дата
				settingStep++;
				lcd.drawFastHLine(10, 74, 108, BLACK);
				if(settingStep>2){
					//Заносим все даты в переменные (день недели, день, месяц);
					EEPROM.write(numWeekDayAddress, numWeekDay);
					EEPROM.write(dayAddress, day);
					EEPROM.write(monthAddress, month);
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
					settingStep=0;
					lcd.clearScreen();
				}
				break;
			case 7:
				settingStep++;
				lcd.drawFastHLine(43, 72, 42, BLACK);
				if(settingStep>1){
					alarmStatus=true;
					EEPROM.write(ACMinute, alarmMinute);
					EEPROM.write(ACHour, alarmHour);
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
					settingStep=0;
					lcd.clearScreen();
				}
				break;
			default://Если не удовлетворяет (обычное меню)
				if(MenuType[MenuChildFirst[MenuLevel]+MenuCurPos]==8){//Не выполняем программу если MenuType=8, так как StatusSettings(Костыль)
					if(MenuLevel==2) alarmStatus=StatusSettings(alarmStatus, true);
					else if(MenuLevel==5){
						bluetoothStatus=StatusSettings(bluetoothStatus, true);
						if(bluetoothStatus){
							Serial.begin(9600);//инциализация Serial
							analogWrite(bluetoothPower, 255);//Включение bt
						}
						else{
							analogWrite(bluetoothPower, 0);//Выключение bt
							Serial.end();//закрытие Serial
						}
					}
					return;
				}
				MenuLevel=MenuChildFirst[MenuLevel]+MenuCurPos;
				MenuCurPos=0;
				lcd.clearScreen();
				break;
		}
	}
	if(pressed(back)&&millisDelay(TimerButton)&&MenuLevel!=0){
		renderingStatics=false;
		switch(MenuType[MenuLevel]){
			case 5://Время
				settingStep--;
				lcd.drawFastHLine(43, 72, 42, BLACK);
				if(settingStep==255){//255 так как byte 0-1=255
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
					settingStep=0;
					lcd.clearScreen();
				}
				break;
			case 6://Дата
				settingStep--;
				lcd.drawFastHLine(10, 74, 108, BLACK);
				if(settingStep==255){//255 так как byte 0-1=255
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
					settingStep=0;
					lcd.clearScreen();
				}
				break;
			case 7:
				settingStep--;
				lcd.drawFastHLine(43, 72, 42, BLACK);
				if(settingStep==255){//255 так как byte 0-1=255
					MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
					MenuLevel=MenuParent[MenuLevel];
					settingStep=0;
					lcd.clearScreen();
				}
				break;
			default:
				MenuCurPos=MenuLevel-MenuChildFirst[MenuParent[MenuLevel]];
				MenuLevel=MenuParent[MenuLevel];
				if(MenuLevel==0) printDates=false;
				lcd.clearScreen();
				break;
		}
	}
	if(pressed(up)&&millisDelay(TimerButton)){
		switch(MenuType[MenuLevel]){
			case 1://Меню
				MenuCurPos--;
				if(MenuCurPos==255) MenuCurPos=MenuChildLast[MenuLevel]-MenuChildFirst[MenuLevel];
				lcd.drawFastVLine(0, 0, 128, BLACK);//Рисуем линию, убираем белый курсор
				break;
			case 3:
				MenuCurPos--;
				if(MenuCurPos<1) MenuCurPos=5;
				lcd.drawFastVLine(0, 0, 128, BLACK);
				break;
			case 4://Яркость
				brightness+=12;
				if(brightness>240) brightness=240;
				fixedSetNum=currentPer;
				analogWrite(backlight, brightness);
				break;
			case 5://Время
				switch(settingStep){
					case 0://Часы
						fixedSetNum=time(4);
						hours++;
						if(hours>23) hours=0;
						lcd.setTextColor(BLACK);
						lcd.setCursor(44,55);
						if(fixedSetNum<10) lcd.print("0");
						lcd.print(fixedSetNum);
						fixedSetNum=time(4);
						break;
					case 1:
						fixedSetNum=time(5);
						minutes++;
						if(minutes>59) minutes=0;
						lcd.setTextColor(BLACK);
						lcd.setCursor(66,55);
						if(fixedSetNum<10) lcd.print("0");
						lcd.print(fixedSetNum);
						fixedSetNum=time(5);
						break;
				}
				break;
			case 6:
				switch(settingStep){
					case 0://День недели
						fixedSetNum=time(3);
						numWeekDay++;
						lcd.setTextColor(BLACK);
						lcd.setCursor(10,56);
						lcd.print(namesDays[fixedSetNum-1]);
						fixedSetNum=time(4);
						break;
					case 1://День
						fixedSetNum=time(2);
						day++;
						if(day>daysinMonths[month]) day=1;
						lcd.setTextColor(BLACK);
						lcd.setCursor(52,56);
						if(fixedSetNum<10) lcd.print("0");
						lcd.print(fixedSetNum);
						fixedSetNum=time(2);
						break;
					case 2://Месяц
						fixedSetNum=time(1);
						month++;
						if(day>daysinMonths[month]){
							lcd.setTextColor(BLACK);
							lcd.setCursor(52,56);
							if(day<10) lcd.print("0");
							lcd.print(day);
							day=daysinMonths[month];
						}
						lcd.setTextColor(BLACK);
						lcd.setCursor(84,56);
						lcd.print(namesMonths[fixedSetNum-1]);
						fixedSetNum=time(1);
						break;
				}
				break;
			case 7://Время
				switch(settingStep){
					case 0://Часы
						fixedSetNum=alarmHour;
						alarmHour++;
						if(alarmHour>23) alarmHour=0;
						lcd.setTextColor(BLACK);
						lcd.setCursor(44,55);
						if(fixedSetNum<10) lcd.print("0");
						lcd.print(fixedSetNum);
						fixedSetNum=alarmHour;
						break;
					case 1:
						fixedSetNum=alarmMinute;
						alarmMinute++;
						if(alarmMinute>59) alarmMinute=0;
						lcd.setTextColor(BLACK);
						lcd.setCursor(66,55);
						if(fixedSetNum<10) lcd.print("0");
						lcd.print(fixedSetNum);
						fixedSetNum=alarmMinute;
						break;
				}
				break;
		}
	}
	if(pressed(down)&&millisDelay(TimerButton)){
		switch(MenuType[MenuLevel]){
			case 1://Меню
				MenuCurPos++;
				lcd.drawFastVLine(0, 0, 128, BLACK);
				if(MenuCurPos>MenuChildLast[MenuLevel]-MenuChildFirst[MenuLevel]) MenuCurPos=0;
				break;
			case 3:
				MenuCurPos++;
				if(MenuCurPos>5) MenuCurPos=1;
				lcd.drawFastVLine(0, 0, 128, BLACK);
				break;
			case 4://Яркость
				brightness-=12;
				if(brightness<12) brightness=12;
				fixedSetNum=currentPer;
				analogWrite(backlight, brightness);
				break;
			case 5://Время
				switch(settingStep){
					case 0://Часы
						fixedSetNum=time(4);
						hours--;
						if(hours==255) hours=23;
						lcd.setTextColor(BLACK);
						lcd.setCursor(44,55);
						if(fixedSetNum<10) lcd.print("0");
						lcd.print(fixedSetNum);
						fixedSetNum=time(4);
						break;
					case 1://Минуты
						fixedSetNum=time(5);
						minutes--;
						if(minutes==255) minutes=59;
						lcd.setTextColor(BLACK);
						lcd.setCursor(66,55);
						if(fixedSetNum<10) lcd.print("0");
						lcd.print(fixedSetNum);
						fixedSetNum=time(5);
						break;
				}
				break;
			case 6://Дата
				switch(settingStep){
					case 0://День недели
						fixedSetNum=time(3);
						numWeekDay--;
						if(numWeekDay<1) numWeekDay=7;
						lcd.setTextColor(BLACK);
						lcd.setCursor(10,56);
						lcd.print(namesDays[fixedSetNum-1]);
						fixedSetNum=time(4);
						break;
					case 1://День
						fixedSetNum=time(2);
						day--;
						if(day<1) day=daysinMonths[month];
						lcd.setTextColor(BLACK);
						lcd.setCursor(52,56);
						if(fixedSetNum<10) lcd.print("0");
						lcd.print(fixedSetNum);
						fixedSetNum=time(2);
						break;
					case 2://Месяц
						fixedSetNum=time(1);
						month--;
						if(month<1) month=12;
						if(day>daysinMonths[month]){//Если дней больше чем в месяце
							lcd.setTextColor(BLACK);
							lcd.setCursor(52,56);
							if(day<10) lcd.print("0");
							lcd.print(day);
							day=daysinMonths[month];
						}
						lcd.setTextColor(BLACK);
						lcd.setCursor(84,56);
						lcd.print(namesMonths[fixedSetNum-1]);
						fixedSetNum=time(1);
						break;
				}
				break;
			case 7://Время
				switch(settingStep){
					case 0://Часы
						fixedSetNum=alarmHour;
						alarmHour--;
						if(alarmHour==255) alarmHour=23;
						lcd.setTextColor(BLACK);
						lcd.setCursor(44,55);
						if(fixedSetNum<10) lcd.print("0");
						lcd.print(fixedSetNum);
						fixedSetNum=alarmHour;
						break;
					case 1://Минуты
						fixedSetNum=alarmMinute;
						alarmMinute--;
						if(alarmMinute==255) alarmMinute=59;
						lcd.setTextColor(BLACK);
						lcd.setCursor(66,55);
						if(fixedSetNum<10) lcd.print("0");
						lcd.print(fixedSetNum);
						fixedSetNum=alarmMinute;
						break;
				}
				break;
		}
	}
	//Костыль
	if(MenuType[MenuChildFirst[MenuLevel]]==8){
		switch(MenuLevel){
			case 2:
				StatusSettings(alarmStatus);
				break;
			case 5:
				StatusSettings(bluetoothStatus);
				break;
		}
	}
	//Использование функции по типу меню
	switch(MenuType[MenuLevel]){
		case 0://Часы
			DigitalClockFace();
			break;
		case 1://Меню
			DrawMenu();
			break;
		case 3://Таймаут экрана
			ListSettings();
			break;
		case 4://Яркость
			BandSettings();
			break;
		case 5://Время
			seconds=0;//обнуляется постоянно
			TimeSettings();
			break;
		case 6://Дата
			DateSettings();
			break;
		case 7:
			AlarmSettings();
			break;
		/*case 8:
			StatusSettings(true);
			break;*/
	}
	//Использование функций по MenuLevel
	switch(MenuLevel){
		case 19:
			analogWrite(backlight, 0);
			resetFunc();
			break;
		case 20:
			if(!renderingStatics){
				lcd.setCursor(26,0);
				lcd.setTextSize(1);
				lcd.print("Information");
				lcd.setCursor(2,16);
				lcd.print("ROM version");
				lcd.setCursor(2,32);
				lcd.print("0.8.1 beta");
				lcd.setCursor(2,48);
				lcd.print("SOC");
				lcd.setCursor(2,64);
				lcd.print("ATmega328p");
				lcd.setCursor(2,80);
				lcd.print("Free RAM");
				lcd.setCursor(2,96);
				lcd.print(freeMemory());
				lcd.print("/2048Kb");
				renderingStatics=true;
			}
			break;
	}
}
