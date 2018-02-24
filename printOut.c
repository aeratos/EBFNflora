#include "digio.h"
#include "delay.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>
#include "lcd_lib.h"
#include <avr/io.h>
#include <avr/pgmspace.h>

#define PC 0
#define LCD 1

volatile int type;

struct UART* uart;

void printString(char* s){
	int l=strlen(s);
	for(int i=0; i<l; ++i, ++s)
		UART_putChar(uart, (uint8_t) *s);
}

void printOut(char* message){
	//type=getOutput();	//prendo da EEPRO 1 se pc 0 se LCD
	//caso Pc
	if(type==PC){
		printString(message);
		printString("\n");
	}
	//caso LCD
	else{
		LCDstring((uint8_t*)message, strlen(message));
	}
}

//prova
int main(void){
	uart=UART_init("uart_0", 115200);
	LCDinit();	//Inizializzazione LCD
	LCDclr();	//Cancella LCD
	LCDcursorOFF();	//Cancella il blink della posizione testuale

	type=PC;
	printOut("ok");
	
	while(1){
		
	}
}
