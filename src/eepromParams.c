#include "eepromParams.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 16

struct UART* uart;

/* GET functions */
uint16_t get_EEPROM_timer(void){
	char eeprom_buffer[BLOCK_SIZE];
	memset(eeprom_buffer, 0, BLOCK_SIZE);
	EEPROM_read(eeprom_buffer, 0, BLOCK_SIZE);
	return atoi(eeprom_buffer);
}

uint16_t get_EEPROM_minLight(void){
	char eeprom_buffer[BLOCK_SIZE];
	memset(eeprom_buffer, 0, BLOCK_SIZE);
	EEPROM_read(eeprom_buffer, 16, BLOCK_SIZE);
	return atoi(eeprom_buffer);
}

uint16_t get_EEPROM_maxLight(void){
	char eeprom_buffer[BLOCK_SIZE];
	memset(eeprom_buffer, 0, BLOCK_SIZE);
	EEPROM_read(eeprom_buffer, 32, BLOCK_SIZE);
	return atoi(eeprom_buffer);
}

uint16_t get_EEPROM_minTemp(void){
	char eeprom_buffer[BLOCK_SIZE];
	memset(eeprom_buffer, 0, BLOCK_SIZE);
	EEPROM_read(eeprom_buffer, 48, BLOCK_SIZE);
	return atoi(eeprom_buffer);
}

uint16_t get_EEPROM_maxTemp(void){
	char eeprom_buffer[BLOCK_SIZE];
	memset(eeprom_buffer, 0, BLOCK_SIZE);
	EEPROM_read(eeprom_buffer, 64, BLOCK_SIZE);
	return atoi(eeprom_buffer);
}

float get_EEPROM_r1(void){
	char eeprom_buffer[BLOCK_SIZE];
	memset(eeprom_buffer, 0, BLOCK_SIZE);
	EEPROM_read(eeprom_buffer, 80, BLOCK_SIZE);
	return atof(eeprom_buffer);
}

float get_EEPROM_c1(void){
	char eeprom_buffer[BLOCK_SIZE];
	memset(eeprom_buffer, 0, BLOCK_SIZE);
	EEPROM_read(eeprom_buffer, 96, BLOCK_SIZE);
	return atof(eeprom_buffer);
}

float get_EEPROM_c2(void){
	char eeprom_buffer[BLOCK_SIZE];
	memset(eeprom_buffer, 0, BLOCK_SIZE);
	EEPROM_read(eeprom_buffer, 112, BLOCK_SIZE);
	return atof(eeprom_buffer);
}

float get_EEPROM_c3(void){
	char eeprom_buffer[BLOCK_SIZE];
	memset(eeprom_buffer, 0, BLOCK_SIZE);
	EEPROM_read(eeprom_buffer, 128, BLOCK_SIZE);
	return atof(eeprom_buffer);
}

uint16_t get_EEPROM_maxPoll(void){
	char eeprom_buffer[BLOCK_SIZE];
	memset(eeprom_buffer, 0, BLOCK_SIZE);
	EEPROM_read(eeprom_buffer, 144, BLOCK_SIZE);
	return atoi(eeprom_buffer);
}

/* SET functions */
void set_EEPROM_timer(uint16_t duration){
	// we write timer on eeprom at address 0
	char str[16];
	sprintf(str, "%d", (int)duration);
	int sizeFrequency = strlen(str)+1;
	EEPROM_write(0, str, sizeFrequency);
}

void set_EEPROM_minLight(uint16_t minLight){
	// we write minLight on eeprom at address 16
	char str[16];
	sprintf(str, "%d", (int)minLight);	
	int sizeMinLight = strlen(str)+1;
	EEPROM_write(16, str, sizeMinLight);
}

void set_EEPROM_maxLight(uint16_t maxLight){
	// we write maxLight on eeprom at address 32
	char str[16];
	sprintf(str, "%d", (int)maxLight);
	int sizeMaxLight = strlen(str)+1;
	EEPROM_write(32, str, sizeMaxLight);
}

void set_EEPROM_minTemp(uint16_t minTemp){
	// we write minTemp on eeprom at address 48
	char str[16];
	sprintf(str, "%d", (int)minTemp);	
	int sizeMinTemp = strlen(str)+1;
	EEPROM_write(48, str, sizeMinTemp);
}

void set_EEPROM_maxTemp(uint16_t maxTemp){
	// we write maxTemp on eeprom at address 64
	char str[16];
	sprintf(str, "%d", (int)maxTemp);	
	int sizeMaxTemp = strlen(str)+1;
	EEPROM_write(64, str, sizeMaxTemp);
}

void set_EEPROM_r1(float R1){
	// we write R1 on eeprom at address 80
	char str[16];
	sprintf(str, "%.1f", R1);	
	int sizeR1 = strlen(str)+1;
	EEPROM_write(80, str, sizeR1);
}

void set_EEPROM_c1(float C1){
	// we write C1 on eeprom at address 96
	char str[16];
	sprintf(str, "%12.7e", C1);
	int sizeC1 = strlen(str)+1;
	EEPROM_write(96, str, sizeC1);
}

void set_EEPROM_c2(float C2){
	// we write C2 on eeprom at address 112
	char str[16];
	sprintf(str, "%12.7e", C2);
	int sizeC2 = strlen(str)+1;
	EEPROM_write(112, str, sizeC2);
}

void set_EEPROM_c3(float C3){
	// we write C3 on eeprom at address 128
	char str[16];
	sprintf(str, "%12.7e", C3);
	int sizeC3 = strlen(str)+1;
	EEPROM_write(128, str, sizeC3);
}

void set_EEPROM_maxPoll(uint16_t maxPoll){
	// we write maxPoll on eeprom at address 144
	char str[16];
	sprintf(str, "%d", (int)maxPoll);
	int sizeMaxPoll = strlen(str)+1;
	EEPROM_write(144, str, sizeMaxPoll);
}
