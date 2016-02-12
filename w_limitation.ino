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

byte current;
byte max;
byte min;
byte ret;

byte limValue(byte current, byte max, byte min = 0){
	if(current>max&&current!=255) return min;
	else if(current<min||current==255) return max;
	else return current;
}

void setup(){
	Serial.begin(9600);
	current=3;
	max=5;
	min=1;
	Serial.println(limValue(current,max,min));
	current--;
	Serial.println(limValue(current,max,min));
}

void loop(){
	current++;
	current=limValue(current,max,min);
	Serial.println(current);
	delay(500);
}
