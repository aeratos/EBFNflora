#include "packet_handler.h"
#include "serial_linux.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STR_LENGTH 16
char inputString[STR_LENGTH];

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

PacketOperations TimerConfigPacket_ops = {
	TIMER_CONFIG_PACKET_TYPE,
	sizeof(TimerConfigPacket),
	TimerConfigPacket_initializeBuffer,
	0,
	0,
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

PacketOperations LightConfigPacket_ops = {
	LIGHT_CONFIG_PACKET_TYPE,
	sizeof(LightConfigPacket),
	LightConfigPacket_initializeBuffer,
	0,
	0,
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

PacketOperations TemperatureConfigPacket_ops = {
	TEMPERATURE_CONFIG_PACKET_TYPE,
	sizeof(TemperatureConfigPacket),
	TemperatureConfigPacket_initializeBuffer,
	0,
	0,
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

PacketOperations PollutionConfigPacket_ops = {
	POLLUTION_CONFIG_PACKET_TYPE,
	sizeof(PollutionConfigPacket),
	PollutionConfigPacket_initializeBuffer,
	0,
	0,
	0
};

void read_from_stdin(void){
	fgets(inputString, STR_LENGTH, stdin);
	if((strlen(inputString) > 0) && (inputString[strlen(inputString) - 1] == '\n'))
		inputString[strlen(inputString) - 1] = '\0';	
}

struct UART* uart;

void flushOutputBuffer(int fd){
	while (packet_handler.tx_size){
		uint8_t c = PacketHandler_txByte(&packet_handler);
		write(fd, &c, 1);
		usleep(1000);
	}
}

int main(int argc, char** argv){
	assert(argc > 1);
	int fd = serial_open(argv[1]);
	if (fd < 0)
		return 0;
	if (serial_set_interface_attribs(fd, 115200, 0) < 0)
		return 0;
	serial_set_blocking(fd, 1); 
	if  (! fd)
		return 0;
	
	PacketHandler_initialize(&packet_handler);
	PacketHandler_installPacket(&packet_handler, &TimerConfigPacket_ops);
	PacketHandler_installPacket(&packet_handler, &LightConfigPacket_ops);
	PacketHandler_installPacket(&packet_handler, &TemperatureConfigPacket_ops);
	PacketHandler_installPacket(&packet_handler, &PollutionConfigPacket_ops);
	
	/* -------------------- TimerConfigPacket -------------------- */
	
	printf("Insert timer duration: ");
	read_from_stdin();
	uint32_t duration = atoi(inputString);
	
	TimerConfigPacket p0 = { {TIMER_CONFIG_PACKET_TYPE, TIMER_CONFIG_PACKET_SIZE, 0}, duration };
	PacketHandler_sendPacket(&packet_handler, (PacketHeader*) &p0);
	flushOutputBuffer(fd);
	
	/* -------------------- LightConfigPacket -------------------- */

	printf("Insert min light: ");
	read_from_stdin();
	uint16_t minLight = atoi(inputString);
	
	printf("Insert max light: ");
	read_from_stdin();
	uint16_t maxLight = atoi(inputString);
	
	LightConfigPacket p1 = { {LIGHT_CONFIG_PACKET_TYPE, LIGHT_CONFIG_PACKET_SIZE, 0}, minLight, maxLight };
	PacketHandler_sendPacket(&packet_handler, (PacketHeader*) &p1);
	flushOutputBuffer(fd);
	
	/* -------------------- TemperatureConfigPacket -------------------- */

	printf("Insert min temperature: ");
	read_from_stdin();
	uint16_t minTemp = atoi(inputString);
	
	printf("Insert max temperature: ");
	read_from_stdin();
	uint16_t maxTemp = atoi(inputString);

	printf("Insert thermistor resistance: ");
	read_from_stdin();
	float r1 = atof(inputString);
	
	float c1 = 1.009249522e-03;
	float c2 = 2.378405444e-04;
	float c3 = 2.019202697e-07;
	
	TemperatureConfigPacket p2 = { {TEMPERATURE_CONFIG_PACKET_TYPE, TEMPERATURE_CONFIG_PACKET_SIZE, 0}, minTemp, maxTemp, r1, c1, c2, c3 };
	PacketHandler_sendPacket(&packet_handler, (PacketHeader*) &p2);
	flushOutputBuffer(fd);
	
	/* -------------------- PollutionConfigPacket -------------------- */
	
	printf("Insert max pollution: ");
	read_from_stdin();
	uint16_t maxPoll = atoi(inputString);
	
	PollutionConfigPacket p3 = { {POLLUTION_CONFIG_PACKET_TYPE, POLLUTION_CONFIG_PACKET_SIZE, 0}, maxPoll };
	PacketHandler_sendPacket(&packet_handler, (PacketHeader*) &p3);
	flushOutputBuffer(fd);
		
	return 0;
}
