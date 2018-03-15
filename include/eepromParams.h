#include "eeprom.h"
#include <stdint.h>

/* GET functions */
uint16_t get_EEPROM_timer(void);
uint16_t get_EEPROM_minLight(void);
uint16_t get_EEPROM_maxLight(void);
uint16_t get_EEPROM_minTemp(void);
uint16_t get_EEPROM_maxTemp(void);
float get_EEPROM_r1(void);
float get_EEPROM_c1(void);
float get_EEPROM_c2(void);
float get_EEPROM_c3(void);
uint16_t get_EEPROM_maxPoll(void);

/* SET functions */
void set_EEPROM_timer(uint16_t duration);
void set_EEPROM_minLight(uint16_t minLight);
void set_EEPROM_maxLight(uint16_t maxLight);
void set_EEPROM_minTemp(uint16_t minTemp);
void set_EEPROM_maxTemp(uint16_t maxTemp);
void set_EEPROM_r1(float R1);
void set_EEPROM_c1(float C1);
void set_EEPROM_c2(float C2);
void set_EEPROM_c3(float C3);
void set_EEPROM_maxPoll(uint16_t maxPoll);
