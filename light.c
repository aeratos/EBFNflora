#include "digio.h"
#include "delay.h"
#include "eepromParams.h"
#include "uart.h"
#include "adc.h"
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>

#define HIGH 1
#define LOW 0

int led = 8;
int buz= 13;
int analog_pin= 5;

struct UART* uart;

volatile uint16_t photocell_value;

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


int main(void){
	
	DigIO_init();
	
	uart=UART_init("uart_0", 115200);
	DigIO_setDirection(led, Output);
	DigIO_setDirection(buz, Output);
	DigIO_setValue(led, LOW);
	DigIO_setValue(buz, LOW);
	
	adc_init();
	
	int min_light= get_EEPROM_minLight();
	int max_light= get_EEPROM_maxLight();
	
	
	while(1){
		photocell_value= adc_read(analog_pin);                      // A5 Analog Port
		char msg[8];
		
		
		//*********DEBUG START*********                       
		sprintf(msg, "Light: %.d \n", photocell_value);
		printString(msg);
		//*********DEBUG END*********

		delayMs(500);
		if(photocell_value<min_light){
			error_blink_sound();
			printString("WARNING: too low light!\t");
			sprintf(msg, "Light: %.d \n", photocell_value);
			printString(msg);
		}
		else if(photocell_value>max_light){
			error_blink_sound();
			printString("WARNING: too high light!\t");
			sprintf(msg, "Light: %.d \n", photocell_value);
			printString(msg);
		}
	}
	
	return 0;
}
