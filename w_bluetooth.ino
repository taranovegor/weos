// Basic Bluetooth sketch HC-05_01
// Sends "Bluetooth Test" to the serial monitor and the software serial once every second.
//
// Connect the HC-05 module and data over Bluetooth
//
// The HC-05 defaults to commincation mode when first powered on.
// The default baud rate for communication is 9600
// Connect the HC-05 TX to Arduino pin 2 RX.
// Connect the HC-05 RX to Arduino pin 3 TX through a voltage divider.
//
#include <SPI.h>
int btData[8] = {0, 0, 0, 0, 0, 0, 0};//8, 16, ...
long buffer;
byte btCycle = 0;
byte btStringLen = 0;
byte btMax = 0;
boolean	handler = false;

void setup(){
	Serial.begin(38400);
}

/*String one = "a";
String two = "b";*/
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

void loop(){
	if(Serial.available()){
		Serial.println(Serial.read());
	}
	/*bluetooth.listen();
	if(bluetooth.isListening()){
		bluetooth.println(bluetooth.read());
		delay(1000);
	}
	/*if(bluetooth.available()){
		buffer=bluetooth.read();
		bluetooth.println(buffer);
		bluetooth.println("OKbbb\r\n");
		/*buffer=ASCII(bluetooth.read());
		if(buffer==46){//если равно '.'
			btCycle++;
			btStringLen=0;
			bluetooth.println("\r\nbtAdd");
			return;
		}
		if(buffer==47){//
			btCycle=0;
			btStringLen=0;
			bluetooth.println(btData[0]);
			bluetooth.println(".");
			bluetooth.println(btData[1]);
			bluetooth.println("\r\nSETTING SET");
			return;
		}
		if(buffer!=46&&buffer!=47){
			btData[btCycle]=buffer;
		}
		bluetooth.println(bluetooth.overflow());
		bluetooth.println("\r\nOK");*/
	//}
}



/*buffer=ASCII(bluetooth.read());
		if(buffer==46){//если равно '.'
			btCycle++;
			btStringLen=0;
		}
		if(buffer==47){//
			btCycle=0;
			btStringLen=0;
			bluetooth.println(btData[0]);
			bluetooth.println("=");
			bluetooth.println(btData[1]);
			bluetooth.println("SETTING SET\r\n");
			return;
		}
		if(buffer!=46&&buffer!=47){
			btData[btCycle]=buffer;
		}
		bluetooth.println("OK\r\n");*/
	/*bluetooth.println("\r\n");
	bluetooth.println(btData[0], DEC);
	bluetooth.println("\r\n");
	bluetooth.println(btData[1], DEC);
	bluetooth.println("\r\n");
	bluetooth.println(btData[2], DEC);
	bluetooth.println("\r\n");
		delay(1000);*/
/*if(btCycle<btMax&&handler==true){
		btCycle++;
		bluetooth.println(btData[btCycle]);
	}
	else if(btCycle==0&&handler==true){
		btCycle=0;
		handler=false;
}*/
/*
/*BTserial.println("1.24");
    delay(1000);
    if(BTserial.available()){
        c[v] = BTserial.read();
        if(BTserial.read()==1){
      }
    }
    else{
    	v=0;
    }
    if(bluetooth.available()&&handler==false){
		btCycle++;
		btData[btCycle] = bluetooth.read();
		if(btData[btCycle]=='/'){
			handler=true;
			btMax=btCycle;
			btCycle=0;
			bluetooth.print("info=");
		}
	}
	if(handler==false) return;
*/