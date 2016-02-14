// Example sketch for interfacing with the DS1302 timekeeping chip.
//
// Copyright (c) 2009, Matt Sparks
// All rights reserved.
//
// http://quadpoint.org/projects/arduino-ds1302
//char buf[50];
//snprintf(buf, sizeof(buf), "%s %04d-%02d-%02d %02d:%02d:%02d",t.day,t.yr, t.mon, t.date,t.hr, t.min, t.sec);
//Serial.println(buf);
//
//
//
//
#include <DS1302.h>

#define kCePin  5
#define kIoPin  6
#define kCLKPin 7

byte fixedSec;

DS1302 rtc(kCePin, kIoPin, kCLKPin);

byte time(byte arg){
	Time t = rtc.time();
	//months = 1 days = 2 numWeekDay = 3 hours = 4 minutes = 5 seconds = 6
	switch(arg){
		case 1://Месяц
			return t.mon;
		case 2://Дата
			return t.day;
		case 3://День недели
			return t.date;
		case 4://Часы
			return t.hr;
		case 5://Минуты
			return t.min;
		case 6://Секунды
			return t.sec;
		default:break;
	}
}

byte resetByte(byte current, byte max){
	if(current>max) return 0;
	else return current;
}

const char* namesDays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

void setup(){
	Serial.begin(9600);
	//rtc.writeProtect(false);
	//rtc.halt(false);
	//год, месяц, день, час, минута, секунда, день недели
	//Time t(2015, 02, 11, 23, 59, 50, 3);
	//rtc.time(t);
	//fixedSec=time(6);
}

void loop(){
	Serial.print(char(50));
	Serial.print(char(50));
	Serial.print(char(56));
	Serial.println(char(37));
	delay(1000);
	/*if(fixedSec!=time(6)){
		Serial.print(namesDays[time(2)]);
		Serial.print(" ");
		Serial.print(time(1));
		Serial.print(".");
		Serial.print(time(3));
		Serial.print("-");
		Serial.print(time(4));
		Serial.print(":");
		Serial.print(time(5));
		Serial.print(":");
		Serial.println(time(6));
		fixedSec=time(6);
	}*/
}

/*Настройки даты*/
//void DateSettings(){
	/*if(!renderingStatics){
		lcd.setTextSize(1);
		lcd.setTextColor(WHITE);
		lcd.setCursor(47,3);
		lcd.print("Date");
		//Текст настроек
		lcd.setCursor(10,56);
		lcd.print(namesDays[time(3)-1]);
		//Дата
		lcd.setCursor(52,56);
		if(time(2)<10) lcd.print("0");
		lcd.print(time(2));
		//Месяц
		lcd.setCursor(84,56);
		lcd.print(namesMonths[time(1)-1]);
		renderingStatics=true;
	}
	switch(settingStep){
		case 0:
			lcd.setCursor(10,56);
			lcd.print(namesDays[time(3)-1]);
			lcd.drawFastHLine(10, 74, 34, WHITE);
			break;
		case 1:
			lcd.setCursor(52,56);
			if(time(2)<10) lcd.print("0");
			lcd.print(time(2));
			lcd.drawFastHLine(52, 74, 22, WHITE);
			break;
		case 2:
			lcd.setCursor(84,56);
			lcd.print(namesMonths[time(1)-1]);
			lcd.drawFastHLine(84, 74, 34, WHITE);
			break;
	}
}

void alarmSettings(){
	if(!renderingStatics){
		lcd.setTextSize(1);
		lcd.setTextColor(WHITE);
		lcd.setCursor(44,3);
		lcd.print("Alarm");
		//Содержимое
		lcd.setCursor(44,55);
		if(alarmHour<10) lcd.print("0");
		lcd.print(alarmHour);
		lcd.print(":");
		if(alarmMinute<10) lcd.print("0");
		lcd.print(alarmMinute);
		renderingStatics=true;
	}
	switch(settingStep){
		case 0:
			lcd.setCursor(44,55);
			if(alarmHour<10) lcd.print("0");
			lcd.print(alarmHour);
			lcd.drawFastHLine(44, 72, 18, WHITE);
			break;
		case 1:
			if(alarmMinute<10) lcd.print("0");
			lcd.print(alarmMinute);
			lcd.drawFastHLine(66, 72, 18, WHITE);
			break;
	}
}*/