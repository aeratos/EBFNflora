#include "adc.h"
#include "delay.h"
#include "uart.h"
#include "eepromParams.h"
#include "digio.h"
#include <stdio.h>
#include <string.h>
#define HIGH 1
#define LOW 0

int led = 8;
int buz= 13;
int analog_pin= 3;

struct UART* uart;

volatile uint16_t air_value;


void printString(char* s){
	int l=strlen(s);
	for(int i=0; i<l; ++i, ++s)
		UART_putChar(uart, (uint8_t) *s);
}

void error_blink_sound(void){
	DigIO_setValue(led, HIGH);
	delayMs(100);
	DigIO_setValue(led, LOW);
	delayMs(100);
	DigIO_setValue(led, HIGH);
	delayMs(100);
	DigIO_setValue(led, LOW);
	delayMs(100);
	
	DigIO_setValue(buz, HIGH);
	delayMs(100);
	DigIO_setValue(buz, LOW);
	delayMs(100);
	DigIO_setValue(buz, HIGH);
	delayMs(100);
	DigIO_setValue(buz, LOW);
}

int main(void) {
	DigIO_init();

	uart=UART_init("uart_0", 115200);
	DigIO_setDirection(led, Output);
	DigIO_setDirection(buz, Output);
	DigIO_setValue(led, LOW);
	DigIO_setValue(buz, LOW);
	
	adc_init();
	
	int maxPoll= get_EEPROM_maxPoll();

	while(1) {
		air_value= adc_read(analog_pin); 
		char msg[16];
		
		if(air_value>maxPoll){
			error_blink_sound();
			sprintf(msg, "sensorValue = %d\n", air_value);
			printString(msg);	
			sprintf(msg, "Polluted air %d\n", air_value);
			printString(msg);
		}
		delayMs(500);
	}
}
