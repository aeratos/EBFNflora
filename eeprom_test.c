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

int main(void){
	EEPROM_init();
	uart = UART_init("uart_0", 115200);
	
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
