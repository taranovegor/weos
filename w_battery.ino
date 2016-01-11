#include <SPI.h>

void setup(){
	Serial.begin(9600);
	analogReference(DEFAULT);
}

float typVbg = 1.179;
/*float readVcc(){
	float result = 0.0;
	float tmp = 0.0;
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	ADCSRA |= _BV(ADSC);//Start conversion
	while (bit_is_set(ADCSRA,ADSC));//measuring
	uint8_t low  = ADCL;//must read ADCL first - it then locks ADCH
	uint8_t high = ADCH;//unlocks both
	tmp = (high<<8) | low;
	tmp = (typVbg * 1023.0) / tmp;
	result = result + tmp;
	return result;
	//result = (result/100)*10+result;//более точно +10%
}*/

float readVcc(){
	float result = 0.0;
	float tmp = 0.0;
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	delay(1);//Здесь пауза
	ADCSRA |= _BV(ADSC);//Start conversion
	while (bit_is_set(ADCSRA,ADSC));//measuring
	uint8_t low  = ADCL;//must read ADCL first - it then locks ADCH
	uint8_t high = ADCH;//unlocks both
	tmp = (high<<8) | low;
	tmp = (typVbg * 1023.0) / tmp;
	result = result + tmp;
	return result;
}

double readCC(){
	double Vcc = 5.0; // не всегда так
	int value = analogRead(0);
	double volt = (value / 1023.0) * Vcc; // правильно только если Vcc = 5.0В
	return volt;
}

float readVHH(){
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	ADCSRA |= _BV(ADSC); // начало преобразований
	while (bit_is_set(ADCSRA, ADSC)); // измерение
	uint8_t low = ADCL; // сначала нужно прочесть ADCL - это запирает ADCH
	uint8_t high = ADCH; // разлочить оба
	float result = (high<<8) | low;
	result = (1.1 * 1023.0) / result; // Результат Vcc в милливольтах
	return result;
}
/*Функция подсчёта питающего напряжения*/
double readVcc(){
	const double typVbg = 1.179;//1.0-1.2v
	double result = 0.0;
	double tmp = 0.0;
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	delayMicroseconds(300);
	ADCSRA |= _BV(ADSC);//Start conversion
	while (bit_is_set(ADCSRA,ADSC));//measuring
	byte low  = ADCL;//must read ADCL first - it then locks ADCH
	byte high = ADCH;//unlocks both
	tmp = (high<<8) | low;
	tmp = (typVbg * 1023.0) / tmp;
	result = result + tmp;
	//result = (result/100)*10+result;//более точно +10%
	voltage = result;//Переносим результат в глобальную переменную voltage
}
void loop(){
	//Serial.println(readVcc());
	Serial.println(readVHH());
}