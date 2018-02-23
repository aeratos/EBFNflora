#include "eeprom.h"
#include <stdint.h>

/* GET functions */
uint8_t get_EEPROM_prescaler(void);
uint16_t get_EEPROM_minLight(void);
uint16_t get_EEPROM_maxLight(void);
uint16_t get_EEPROM_minTemp(void);
uint16_t get_EEPROM_maxTemp(void);
uint16_t get_EEPROM_maxPoll(void);
uint32_t get_EEPROM_r1(void);
float get_EEPROM_c1(void);
float get_EEPROM_c2(void);
float get_EEPROM_c3(void);

/* SET functions */
void set_EEPROM_prescaler(char* str);
void set_EEPROM_minLight(char* str);
void set_EEPROM_maxLight(char* str);
void set_EEPROM_minTemp(char* str);
void set_EEPROM_maxTemp(char* str);
void set_EEPROM_maxPoll(char* str);
void set_EEPROM_r1(char* str);
void set_EEPROM_c1(char* str);
void set_EEPROM_c2(char* str);
void set_EEPROM_c3(char* str);
