#include "adc.h"
#include "delay.h"
#include "digio.h"
#include "eepromParams.h"
#include "lcd_lib.h"
#include "timer.h"
#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define HIGH 1
#define LOW 0

#define BUFFER_SIZE 16

struct UART* uart;

int led = 8;
int buz= 9;
const int buttonLeft = 2;  //PD2 is the port related to INT0
const int buttonRight = 3; //PD3 is the port related to INT1
int temp_pin = 0;
int poll_pin = 3;
int light_pin = 5;

volatile int reset;
volatile int i = 0; //i=1 light, i=2 temperature, i=3 air quality
volatile int waitingForOutput = 1;

uint16_t timer;
uint16_t minLight;
uint16_t maxLight;
uint16_t minTemp;
uint16_t maxTemp;
uint16_t maxPoll;
float R1;
float C1;
float C2;
float C3;

void printString(char* s){
	int l=strlen(s);
	for(int i=0; i<l; ++i, ++s)
		UART_putChar(uart, (uint8_t) *s);
}

void printOnLCD(char* message){
	LCDstring((uint8_t*)message, strlen(message));
}

void readString(char* dest, int size){ 
	int i = 0;
	while(1){
		uint8_t c = UART_getChar(uart);
		dest[i++] = c;
		dest[i] = 0;
		if(i == size-1){  //end of dest buffer
			while(c != 0) c = UART_getChar(uart); //read all incoming chars without storing in dest buffer: they are lost
			return;
		}
		else if(c=='\n' || c==0) return;
	}
}

ISR(INT0_vect){ //external interrupt service routine
	if(waitingForOutput && DigIO_getValue(buttonLeft) == HIGH){
		reset = 1;
		waitingForOutput = 0;
	}
}

ISR(INT1_vect){ //external interrupt service routine
	if(waitingForOutput && DigIO_getValue(buttonRight) == HIGH){
		reset = 0;
		waitingForOutput = 0;
	}
}

void error_blink_sound(void){
	DigIO_setValue(led, HIGH);
	delayMs(50);
	DigIO_setValue(led, LOW);
	delayMs(50);
	DigIO_setValue(led, HIGH);
	delayMs(50);
	DigIO_setValue(led, LOW);
	delayMs(50);
	
	DigIO_setValue(buz, HIGH);
	delayMs(50);
	DigIO_setValue(buz, LOW);
	delayMs(50);
	DigIO_setValue(buz, HIGH);
	delayMs(50);
	DigIO_setValue(buz, LOW);
}

void controlLight(void){
	uint16_t photocell_value= adc_read(light_pin); 
	char msg[BUFFER_SIZE];
	
	if(photocell_value<minLight){
		printOnLCD("LOW light");
		sprintf(msg, "Light: %d", photocell_value);
		LCDGotoXY(0,1);
		printOnLCD(msg);
		error_blink_sound();
	}
	else if(photocell_value>maxLight){
		printOnLCD("HIGH light");
		sprintf(msg, "Light: %d", photocell_value);
		LCDGotoXY(0,1);
		printOnLCD(msg);
		error_blink_sound();
	}
}

void controlTemp(void){	
	float logR2, R2, T, Tc;
	uint16_t Vo = adc_read(temp_pin);
	//current thermistor resistance 
	R2 = R1 * (1023.0 / (float)Vo - 1.0);
	logR2 = log(R2);
	T = (1.0 / (C1 + C2*logR2 + C3*logR2*logR2*logR2));
	Tc = T - 273.15;
	
	char msg[BUFFER_SIZE];
	
	if(Tc<minTemp){
		printOnLCD("LOW temp");
		sprintf(msg, "Temp: %.1f C", Tc);
		LCDGotoXY(0,1);
		printOnLCD(msg);
		error_blink_sound();	
	}else if(Tc>maxTemp){
		printOnLCD("HIGH temp");
		sprintf(msg, "Temp: %.1f C", Tc);
		LCDGotoXY(0,1);
		printOnLCD(msg);
		error_blink_sound();
	}
}

void controlPoll(void){
	uint16_t air_value= adc_read(poll_pin); 
	char msg[BUFFER_SIZE];
		
	if(air_value>maxPoll){
		sprintf(msg, "Polluted air %d", air_value);
		printOnLCD(msg);
		sprintf(msg, "Air: %d", air_value);
		LCDGotoXY(0,1);
		printOnLCD(msg);
		error_blink_sound();
	}
}

void timerFn(void* args){
	LCDclr();
	if(i==0){
		controlLight();
		i++;
		return;
	}
	else if(i==1){
		controlTemp();
		i++;
		return;
	}
	else if(i==2){
		controlPoll();
		i = 0;
		return;
	}
}

int main(void){	
	uart = UART_init("uart_0", 115200);
	Timers_init();
	
	DigIO_init();
	DigIO_setDirection(led, Output);
	DigIO_setDirection(buz, Output);
	DigIO_setValue(led, LOW);
	DigIO_setValue(buz, LOW);
	
	DigIO_setDirection(buttonLeft, Input); //PD2 as an input
	DigIO_setValue(buttonLeft, HIGH);
	DigIO_setDirection(buttonRight, Input); //PD3 as an input
	DigIO_setValue(buttonRight, HIGH);
	
	LCDinit();
	LCDclr();
	LCDcursorOFF();
	
	adc_init();
	
	EIMSK = (1<<INT1)|(1<<INT0); //Turn ON INT0 and INT1
	EICRA = (1<<ISC11)|(1<<ISC10)|(1<<ISC01)|(1<<ISC00);
	sei(); //enable interrupts globally
	
	//waiting for output setup
	while(waitingForOutput){
		LCDclr();
		printOnLCD("Setup sensors?");
		LCDGotoXY(0,1);
		printOnLCD("[Y/n]?");
		delayMs(300);
	}
	
	LCDclr();
	
	if(reset){
		char rx_message[BUFFER_SIZE];
		
		printString("timer in ms?\n");
		readString(rx_message, BUFFER_SIZE);
		set_EEPROM_timer(rx_message);
		
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
	
	timer = get_EEPROM_timer();
	minLight = get_EEPROM_minLight();
	maxLight = get_EEPROM_maxLight();
	minTemp = get_EEPROM_minTemp();
	maxTemp = get_EEPROM_maxTemp();
	maxPoll = get_EEPROM_maxPoll();
	R1 = get_EEPROM_r1();
	C1 = get_EEPROM_c1();
	C2 = get_EEPROM_c2();
	C3 = get_EEPROM_c3();
	
	struct Timer* timerSensors = Timer_create("timer_0", timer, timerFn, NULL); 
	Timer_start(timerSensors);
	
	while(1){}
	
	return 0;
}
