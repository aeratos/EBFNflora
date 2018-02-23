#include "eepromParams.h"
#include "delay.h"
#include "uart.h"
#include <string.h>
#include <stdio.h>

struct UART* uart;

void printString(char* s){
	int l=strlen(s);
	for(int i=0; i<l; ++i, ++s)
		UART_putChar(uart, (uint8_t) *s);
}

char* frequency = "125K";
char* minLight = "50";
char* maxLight = "700";
char* minTemp = "15";
char* maxTemp  = "55";
char* maxPoll = "150";
char* R1 = "13000";
char* C1 = "1.009249522e-03";
char* C2 = "2.378405444e-04";
char* C3 = "2.019202697e-07";

int main(void){
	EEPROM_init();
	uart = UART_init("uart_0",115200);
	
	set_EEPROM_prescaler(frequency);
	set_EEPROM_minLight(minLight);
	set_EEPROM_maxLight(maxLight);
	set_EEPROM_minTemp(minTemp);
	set_EEPROM_maxTemp(maxTemp);
	set_EEPROM_maxPoll(maxPoll);
	set_EEPROM_r1(R1);
	set_EEPROM_c1(C1);
	set_EEPROM_c2(C2);
	set_EEPROM_c3(C3);
	
	while(1){
		char msg[16];
		
		int fClock = get_EEPROM_prescaler();
		sprintf(msg, "%d\n", fClock);
		printString(msg);
		delayMs(300);
		
		int minLightValue = get_EEPROM_minLight();
		sprintf(msg, "%d\n", minLightValue);
		printString(msg);
		delayMs(300);
		
		int maxLightValue = get_EEPROM_maxLight();
		sprintf(msg, "%d\n", maxLightValue);
		printString(msg);
		delayMs(300);
		
		int minTempValue = get_EEPROM_minTemp();
		sprintf(msg, "%d\n", minTempValue);
		printString(msg);
		delayMs(300);
		
		int maxTempValue = get_EEPROM_maxTemp();
		sprintf(msg, "%d\n", maxTempValue);
		printString(msg);
		delayMs(300);
		
		int maxPollValue = get_EEPROM_maxPoll();
		sprintf(msg, "%d\n", maxPollValue);
		printString(msg);
		delayMs(300);
		
		int r1Value = get_EEPROM_r1();
		sprintf(msg, "%d\n", r1Value);
		printString(msg);
		delayMs(300);
		
		float c1Value = get_EEPROM_c1();
		sprintf(msg, "%f\n", c1Value);
		printString(msg);
		delayMs(300);
		
		float c2Value = get_EEPROM_c2();
		sprintf(msg, "%f\n", c2Value);
		printString(msg);
		delayMs(300);
		
		float c3Value = get_EEPROM_c3();
		sprintf(msg, "%f\n", c3Value);
		printString(msg);
		delayMs(300);
	}
}
