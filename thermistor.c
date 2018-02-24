#include "adc.h"
#include "delay.h"
#include "uart.h"
#include "digio.h"
#include "eepromParams.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define HIGH 1
#define LOW 0

int ThermistorPin = 0;
int Vo;

int led = 8;
int buz= 13;

struct UART* uart;

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
	
	// initialize adc
	adc_init();
	
	float R1 = get_EEPROM_r1();
	float logR2, R2, T, Tc;
	float c1 = get_EEPROM_c1(); 
	float c2 = get_EEPROM_c2();
	float c3 = get_EEPROM_c3();
	float MinTemp= get_EEPROM_minTemp();
	float MaxTemp= get_EEPROM_maxTemp();
	
	
	while(1){
		Vo = adc_read(ThermistorPin);
		//resistenza attuale del termistore 
		R2 = R1 * (1023.0 / (float)Vo - 1.0);
		logR2 = log(R2);
		T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
		Tc = T - 273.15;
		
		char msg[16];	
	
		if(Tc<MinTemp){
			error_blink_sound();
	 		printString("Ambiente freddo\n");
			sprintf(msg, "Temp: %.1f C\n", Tc);
			printString(msg);		
	 	}else if(Tc>MaxTemp){
			error_blink_sound();
	 		printString("Ambiente caldo\n");
			sprintf(msg, "Temp: %.1f C\n", Tc);
			printString(msg);
	 	}
	 	
	 	delayMs(500);
	}
	
	return 0;
}
