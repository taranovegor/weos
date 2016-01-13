
int btBuffer;
int btData[4] = {0, 0, 0, 0};//4, 8, 16, ...
byte btDataCycle = 0;
byte btDataLen = 0;
boolean btHandler = false;
byte minutes = 32;
byte hours = 22;

byte ASCII(byte arg){//Функция перевода цифр из ASCII в DEC
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

void setup(){
	Serial.begin(9600);
}


void bluetooth(){
	if(Serial.available()){
		btBuffer=Serial.read();
		switch(btBuffer){
			case 63:
				Serial.println(hours);
				Serial.print(":");
				Serial.println(minutes);
				break;
			case 46:
				btDataCycle++;
				btDataLen=0;
				break;
			case 59://Если передан символ ';' означающий конец комманды
				btBuffer=0;
				btDataCycle=0;
				btHandler=true;
				Serial.print("SETTING TRANSFERRED\n\r");
				Serial.println(btData[0]);
				Serial.print(".");
				Serial.println(btData[1]);
				btRequestHandler();
				break;
			default:
				if(btDataCycle==0){
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


void btRequestHandler(){
	switch(btData[0]){
		case 1:
			minutes=btData[1];
		break;
		case 2:
			hours=btData[1];
		break;
	}
	btData[0]=0;
	btData[1]=0;
}

void loop(){
	bluetooth();
}
