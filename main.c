#include "adc.h"
#include "delay.h"
#include "digio.h"
#include "eepromParams.h"
#include "lcd_lib.h"
#include "packet_handler.h"
#include "timer.h"
#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define HIGH 1
#define LOW 0

#define BUFFER_SIZE 16

struct UART* uart;

int led = 8;
int buz= 9;
int buttonLeft = 2;  //PD2 is the port related to INT0
int buttonRight = 3; //PD3 is the port related to INT1
int temp_pin = 0;
int poll_pin = 3;
int light_pin = 5;

volatile int reset;
volatile int i = 0; //i=1 light, i=2 temperature, i=3 air quality
volatile int waitingForOutput = 1;
volatile int EEPROM_params_ready = 0;

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

void printOnLCD(char* message){
	LCDstring((uint8_t*)message, strlen(message));
}

ISR(INT0_vect){ //external interrupt service routine
	if(waitingForOutput && DigIO_getValue(buttonLeft) == HIGH){
		reset = 1;
		waitingForOutput = 0;
	}
	else if(!waitingForOutput && !EEPROM_params_ready && DigIO_getValue(buttonLeft) == HIGH){
		EEPROM_params_ready = 1;
	}
}

ISR(INT1_vect){ //external interrupt service routine
	if(waitingForOutput && DigIO_getValue(buttonRight) == HIGH){
		reset = 0;
		waitingForOutput = 0;
	}
	else if(!waitingForOutput && !EEPROM_params_ready && DigIO_getValue(buttonRight) == HIGH){
		EEPROM_params_ready = 1;
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
		sprintf(msg, "Polluted air");
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

PacketHandler packet_handler;

/* -------------------- TimerConfigPacket -------------------- */

#pragma pack(push, 1)
typedef struct TimerConfigPacket{
	PacketHeader header;
	uint32_t duration;
} TimerConfigPacket;
#pragma pack(pop)

#define TIMER_CONFIG_PACKET_TYPE 0
#define TIMER_CONFIG_PACKET_SIZE (sizeof(TimerConfigPacket))

TimerConfigPacket timer_config_packet_buffer;

PacketHeader* TimerConfigPacket_initializeBuffer(PacketType type, PacketSize size, void* args __attribute__((unused))) {
	if (type != TIMER_CONFIG_PACKET_TYPE || size != TIMER_CONFIG_PACKET_SIZE)
		return 0;
	return (PacketHeader*) &timer_config_packet_buffer;
}

PacketStatus TimerConfigPacket_onReceive(PacketHeader* header, void* args __attribute__((unused))){
	TimerConfigPacket* p = (TimerConfigPacket*) header;
	int duration = p->duration;
	set_EEPROM_timer(duration);
	char buffer[8];
	sprintf(buffer, "%d", duration);
	LCDclr();
	printOnLCD(buffer);
	return Success;
}

PacketOperations TimerConfigPacket_ops = {
	TIMER_CONFIG_PACKET_TYPE,
	sizeof(TimerConfigPacket),
	TimerConfigPacket_initializeBuffer,
	0,
	TimerConfigPacket_onReceive,
	0
};

/* -------------------- LightConfigPacket -------------------- */

#pragma pack(push, 1)
typedef struct LightConfigPacket{
	PacketHeader header;
	uint16_t minLight;
	uint16_t maxLight;
} LightConfigPacket;
#pragma pack(pop)

#define LIGHT_CONFIG_PACKET_TYPE 1
#define LIGHT_CONFIG_PACKET_SIZE (sizeof(LightConfigPacket))

LightConfigPacket light_config_packet_buffer;

PacketHeader* LightConfigPacket_initializeBuffer(PacketType type, PacketSize size, void* args __attribute__((unused))) {
	if (type != LIGHT_CONFIG_PACKET_TYPE || size != LIGHT_CONFIG_PACKET_SIZE)
		return 0;
	return (PacketHeader*) &light_config_packet_buffer;
}

PacketStatus LightConfigPacket_onReceive(PacketHeader* header, void* args __attribute__((unused))){
	LightConfigPacket* p = (LightConfigPacket*) header;
	int minLight = p->minLight;
	int maxLight = p->maxLight;
	set_EEPROM_minLight(minLight);
	set_EEPROM_maxLight(maxLight);
	char buffer[16];
	sprintf(buffer, "%d, %d", minLight, maxLight);
	LCDclr();
	printOnLCD(buffer);
	return Success;
}

PacketOperations LightConfigPacket_ops = {
	LIGHT_CONFIG_PACKET_TYPE,
	sizeof(LightConfigPacket),
	LightConfigPacket_initializeBuffer,
	0,
	LightConfigPacket_onReceive,
	0
};

/* -------------------- TemperatureConfigPacket -------------------- */

#pragma pack(push, 1)
typedef struct TemperatureConfigPacket{
	PacketHeader header;
	uint16_t minTemp;
	uint16_t maxTemp;
	float r1;
	float c1;
	float c2;
	float c3;
} TemperatureConfigPacket;
#pragma pack(pop)

#define TEMPERATURE_CONFIG_PACKET_TYPE 2
#define TEMPERATURE_CONFIG_PACKET_SIZE (sizeof(TemperatureConfigPacket))

TemperatureConfigPacket temperature_config_packet_buffer;

PacketHeader* TemperatureConfigPacket_initializeBuffer(PacketType type, PacketSize size, void* args __attribute__((unused))) {
	if (type != TEMPERATURE_CONFIG_PACKET_TYPE || size != TEMPERATURE_CONFIG_PACKET_SIZE)
		return 0;
	return (PacketHeader*) &temperature_config_packet_buffer;
}

PacketStatus TemperatureConfigPacket_onReceive(PacketHeader* header, void* args __attribute__((unused))){
	TemperatureConfigPacket* p = (TemperatureConfigPacket*) header;
	int minTemp = p->minTemp;
	int maxTemp = p->maxTemp;
	float r1 = p->r1;
	float c1 = p->c1;
	float c2 = p->c2;
	float c3 = p->c3;
	set_EEPROM_minTemp(minTemp);
	set_EEPROM_maxTemp(maxTemp);
	set_EEPROM_r1(r1);
	set_EEPROM_c1(c1);
	set_EEPROM_c2(c2);
	set_EEPROM_c3(c3);
	char buffer[16];
	sprintf(buffer, "%d, %d", minTemp, maxTemp);
	LCDclr();
	printOnLCD(buffer);
	sprintf(buffer, "%.1f", r1);
	LCDGotoXY(0,1);
	printOnLCD(buffer);;
	return Success;
}

PacketOperations TemperatureConfigPacket_ops = {
	TEMPERATURE_CONFIG_PACKET_TYPE,
	sizeof(TemperatureConfigPacket),
	TemperatureConfigPacket_initializeBuffer,
	0,
	TemperatureConfigPacket_onReceive,
	0
};

/* -------------------- PollutionConfigPacket -------------------- */

#pragma pack(push, 1)
typedef struct PollutionConfigPacket{
	PacketHeader header;
	uint16_t maxPoll;
} PollutionConfigPacket;
#pragma pack(pop)

#define POLLUTION_CONFIG_PACKET_TYPE 3
#define POLLUTION_CONFIG_PACKET_SIZE (sizeof(PollutionConfigPacket))

PollutionConfigPacket pollution_config_packet_buffer;

PacketHeader* PollutionConfigPacket_initializeBuffer(PacketType type, PacketSize size, void* args __attribute__((unused))) {
	if (type != POLLUTION_CONFIG_PACKET_TYPE || size != POLLUTION_CONFIG_PACKET_SIZE)
		return 0;
	return (PacketHeader*) &pollution_config_packet_buffer;
}

PacketStatus PollutionConfigPacket_onReceive(PacketHeader* header, void* args __attribute__((unused))){
	PollutionConfigPacket* p = (PollutionConfigPacket*) header;
	int maxPoll = p->maxPoll;
	set_EEPROM_maxPoll(maxPoll);
	char buffer[8];
	sprintf(buffer, "%d", maxPoll);
	LCDclr();
	printOnLCD(buffer);
	return Success;
}

PacketOperations PollutionConfigPacket_ops = {
	POLLUTION_CONFIG_PACKET_TYPE,
	sizeof(PollutionConfigPacket),
	PollutionConfigPacket_initializeBuffer,
	0,
	PollutionConfigPacket_onReceive,
	0
};

void flushInputBuffers(void){
	while (UART_rxBufferFull(uart)){
		uint8_t c = UART_getChar(uart);
		PacketHandler_rxByte(&packet_handler, c);
	}
}

int main(void){	
	uart = UART_init("uart_0", 115200);
	
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
	
	//getting last set params in EEPROM for sensing temperature in setup loop
	minTemp = get_EEPROM_minTemp();
	maxTemp = get_EEPROM_maxTemp();
	R1 = get_EEPROM_r1();
	C1 = get_EEPROM_c1();
	C2 = get_EEPROM_c2();
	C3 = get_EEPROM_c3();
	
	EIMSK = (1<<INT1)|(1<<INT0); //turn ON INT0 and INT1
	EICRA = (1<<ISC11)|(1<<ISC10)|(1<<ISC01)|(1<<ISC00);
	
	printOnLCD("Hello");
	LCDGotoXY(0,1);
	printOnLCD("EBFNflora");
	delayMs(1500);
	
	sei(); //enable interrupts globally
	
	//waiting for output setup
	while(waitingForOutput){
		LCDclr();
		printOnLCD("Setup sensors?");
		
		float logR2, R2, T, Tc;
		uint16_t Vo = adc_read(temp_pin);
		//current thermistor resistance 
		R2 = R1 * (1023.0 / (float)Vo - 1.0);
		logR2 = log(R2);
		T = (1.0 / (C1 + C2*logR2 + C3*logR2*logR2*logR2));
		Tc = T - 273.15;
	
		char msg[BUFFER_SIZE];
		sprintf(msg, "Temp: %.1f C", Tc);
		LCDGotoXY(0,1);
		printOnLCD(msg);
		
		delayMs(500);
	}
	
	LCDclr();
	
	if(reset){ //reset = 1 if the pushbutton related to INT0 has been pressed
		PacketHandler_initialize(&packet_handler);
		PacketHandler_installPacket(&packet_handler, &TimerConfigPacket_ops);
		PacketHandler_installPacket(&packet_handler, &LightConfigPacket_ops);
		PacketHandler_installPacket(&packet_handler, &TemperatureConfigPacket_ops);
		PacketHandler_installPacket(&packet_handler, &PollutionConfigPacket_ops);
		
		while(!EEPROM_params_ready){ //break if any pushbutton is pressed
			flushInputBuffers();
		}
		PacketHandler_uninstallPacket(&packet_handler, TIMER_CONFIG_PACKET_TYPE);
		PacketHandler_uninstallPacket(&packet_handler, LIGHT_CONFIG_PACKET_TYPE);
		PacketHandler_uninstallPacket(&packet_handler, TEMPERATURE_CONFIG_PACKET_TYPE);
		PacketHandler_uninstallPacket(&packet_handler, POLLUTION_CONFIG_PACKET_TYPE);
		LCDclr();
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

	//disable interrupts globally to turn OFF INT0 and INT1
	cli();
	EIMSK = 0x00;
	EICRA = 0x00;
	Timers_init(); //initialize timer
	sei(); //enable interrupts globally
	
	struct Timer* timerSensors = Timer_create("timer_0", timer, timerFn, NULL); 
	Timer_start(timerSensors);
	
	while(1);
	
	return 0;
}
