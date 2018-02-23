#include "digio.h"
#include "eepromParams.h"
#include "lcd_lib.h"
#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

#define HIGH 1
#define LOW 0

#define MAGRA 0
#define GRASSA 1
#define PC 0
#define LCD 1

#define BUFFER_SIZE 16

struct UART* uart;

const int buttonLeft = 2;  //PD2 is the port related to INT0
const int buttonRight = 3; //PD3 is the port related to INT1

volatile int type;
volatile int output;

char* selectPlant = "Magra o grassa?";
volatile int waitingForPlant = 1;

char* selectOutput = "PC o LCD?";
volatile int waitingForOutput = 1;

void printString(char* s){
	int l=strlen(s);
	for(int i=0; i<l; ++i, ++s)
		UART_putChar(uart, (uint8_t) *s);
}

void readString(char* dest, int size){ 
	int i = 0;
	while(1){
		uint8_t c = UART_getChar(uart);
		dest[i++] = c;
		dest[i] = 0;
		if(i == size-1){  // end of dest buffer
			while(c != 0) c = UART_getChar(uart); // read all incoming chars without storing in dest buffer: they are lost
			return;
		}
		else if(c=='\n' || c==0) return;
	}
}

ISR(INT0_vect){ /* external interrupt service routine */
	if(waitingForPlant && DigIO_getValue(buttonLeft) == HIGH){
		type = MAGRA;
		waitingForPlant = 0;
	}
	else if(waitingForOutput && DigIO_getValue(buttonLeft) == HIGH){
		output = PC;
		waitingForOutput = 0;
	}
}

ISR(INT1_vect){ /* external interrupt service routine */
	if(waitingForPlant && DigIO_getValue(buttonRight) == HIGH){
		type = GRASSA;
		waitingForPlant = 0;
	}
	else if(waitingForOutput && DigIO_getValue(buttonRight) == HIGH){
		output = LCD;
		waitingForOutput = 0;
	}
}


int main(void){	
	uart = UART_init("uart_0", 115200);
	
	DigIO_setDirection(buttonLeft, Input); // PD2 as an input
	DigIO_setValue(buttonLeft, HIGH);
	DigIO_setDirection(buttonRight, Input); // PD3 as an input
	DigIO_setValue(buttonRight, HIGH);
	
	EIMSK = (1<<INT1)|(1<<INT0); // Turn ON INT0 and INT1
	EICRA = (1<<ISC11)|(1<<ISC10)|(1<<ISC01)|(1<<ISC00);
	
	sei(); // enable interrupts globally
	
	LCDinit();
	LCDclr();
	LCDcursorOFF();
	LCDstring((uint8_t*)selectPlant, strlen(selectPlant));
	while(waitingForPlant);
	
	LCDclr();
	LCDstring((uint8_t*)selectOutput, strlen(selectOutput));
	while(waitingForOutput);
	
	LCDclr();
	
	if (type == PC) {
		char rx_message[BUFFER_SIZE];
		
		printString("frequency?\n");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_prescaler(rx_message);
		
		printString("min light?\n");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_minLight(rx_message);
		
		printString("max light?\n");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_maxLight(rx_message);
		
		printString("min temperature?\n");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_minTemp(rx_message);
		
		printString("max temperature?\n");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_maxTemp(rx_message);
	
		printString("max pollution?\n");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_maxPoll(rx_message);
	
		printString("thermistor resistance?\n");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_r1(rx_message);
	}
	
	while(1);
}
