#include "adc.h"
#include "delay.h"
#include "digio.h"
#include "eepromParams.h"
#include "lcd_lib.h"
#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define HIGH 1
#define LOW 0

#define PC 0
#define LCD 1

#define BUFFER_SIZE 32

struct UART* uart;

int led = 8;
int buz= 13;
const int buttonLeft = 2;  //PD2 is the port related to INT0
const int buttonRight = 3; //PD3 is the port related to INT1
int temp_pin = 0;
int poll_pin = 3;
int light_pin = 5;

volatile int output;

uint8_t prescaler;
uint16_t minLight;
uint16_t maxLight;
uint16_t minTemp;
uint16_t maxTemp;
uint16_t maxPoll;
float R1;
float C1;
float C2;
float C3;

const char* selectOutput = "PC o LCD?";
volatile int waitingForOutput = 1;

void printString(char* s){
	int l=strlen(s);
	for(int i=0; i<l; ++i, ++s)
		UART_putChar(uart, (uint8_t) *s);
}

void printOut(char* message){
	if(output==PC){
		printString(message);
		printString("\n");
	}
	else{
		LCDstring((uint8_t*)message, strlen(message));
	}
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
	if(waitingForOutput && DigIO_getValue(buttonLeft) == HIGH){
		output = PC;
		waitingForOutput = 0;
	}
}

ISR(INT1_vect){ /* external interrupt service routine */
	if(waitingForOutput && DigIO_getValue(buttonRight) == HIGH){
		output = LCD;
		waitingForOutput = 0;
	}
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

void controlLight(void){
	uint16_t photocell_value= adc_read(light_pin); 
	char msg[BUFFER_SIZE];
	
	if(photocell_value<minLight){
		error_blink_sound();
		printOut("LOW light");
		sprintf(msg, "Light: %d", photocell_value);
		if(output==LCD){
			LCDGotoXY(0,1);
		}
		printOut(msg);
	}
	else if(photocell_value>maxLight){
		error_blink_sound();
		printOut("HIGH light");
		sprintf(msg, "Light: %d", photocell_value);
		if(output==LCD){
			LCDGotoXY(0,1);
		}
		printOut(msg);
	}
}

void controlTemp(void){	
	R1 = get_EEPROM_r1();
	float logR2, R2, T, Tc;
	C1 = get_EEPROM_c1(); 
	C2 = get_EEPROM_c2();
	C3 = get_EEPROM_c3();
	minTemp= get_EEPROM_minTemp();
	maxTemp= get_EEPROM_maxTemp();
	uint16_t Vo = adc_read(temp_pin);
	//current thermistor resistance 
	R2 = R1 * (1023.0 / (float)Vo - 1.0);
	logR2 = log(R2);
	T = (1.0 / (C1 + C2*logR2 + C3*logR2*logR2*logR2));
	Tc = T - 273.15;
		
	char msg[BUFFER_SIZE];
	
	if(Tc<minTemp){
		error_blink_sound();
		printOut("LOW temp");
		sprintf(msg, "Temp: %.1f C", Tc);
		if(output==LCD){
			LCDGotoXY(0,1);
		}	
		printOut(msg);		
	}else if(Tc>maxTemp){
		error_blink_sound();
		printOut("HIGH temp");
		sprintf(msg, "Temp: %.1f C", Tc);
		if(output==LCD){
			LCDGotoXY(0,1);
		}
		printOut(msg);
	}
}

void controlPoll(void){
	uint16_t air_value= adc_read(poll_pin); 
	char msg[BUFFER_SIZE];
		
	if(air_value>maxPoll){
		error_blink_sound();
		sprintf(msg, "Polluted air %d", air_value);
		printOut(msg);
		sprintf(msg, "Air: = %d", air_value);
		if(output==LCD){
			LCDGotoXY(0,1);
		}
		printOut(msg);
	}
}

int main(void){	
	uart = UART_init("uart_0", 115200);
	
	DigIO_init();
	DigIO_setDirection(led, Output);
	DigIO_setDirection(buz, Output);
	DigIO_setValue(led, LOW);
	DigIO_setValue(buz, LOW);
	
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
	LCDstring((uint8_t*)selectOutput, strlen(selectOutput));
	while(waitingForOutput);
	
	LCDclr();
	
	if(output == PC){
		char rx_message[BUFFER_SIZE];
		
		printOut("frequency?");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_prescaler(rx_message);
		
		printOut("min light?");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_minLight(rx_message);
		
		printOut("max light?");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_maxLight(rx_message);
		
		printOut("min temperature?");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_minTemp(rx_message);
	
		printOut("max temperature?");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_maxTemp(rx_message);
	
		printOut("max pollution?");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_maxPoll(rx_message);
	
		printOut("thermistor resistance?");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_r1(rx_message);
	}
	
	prescaler = get_EEPROM_prescaler();
	minLight = get_EEPROM_minLight();
	maxLight = get_EEPROM_maxLight();
	minTemp = get_EEPROM_minTemp();
	maxTemp = get_EEPROM_maxTemp();
	maxPoll = get_EEPROM_maxPoll();
	R1 = get_EEPROM_r1();
	C1 = get_EEPROM_c1();
	C2 = get_EEPROM_c2();
	C3 = get_EEPROM_c3();
	
	adc_init_with_prescaler(prescaler);
	
	while(1){
		//LCDclr();
		//controlLight();
		//delayMs(300);
		//LCDclr();
		//controlTemp();
		//delayMs(300);
		LCDclr();
		controlPoll();
		delayMs(1000);
	}
}
