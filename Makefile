PREFIX=.
CC=avr-gcc
CC_OPTS=-Wall --std=gnu99 -DF_CPU=16000000UL  -O3 -funsigned-char -funsigned-bitfields  -fshort-enums -Wall -Wstrict-prototypes -mmcu=atmega328 -Wl,-u,vfprintf -lprintf_flt -lm -I$(PREFIX)/include

AVRDUDE=avrdude

# com1 = serial port. Use lpt1 to connect to parallel port.
AVRDUDE_PORT = /dev/ttyACM0    # programmer connected to serial device
AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET)
AVRDUDE_FLAGS = -p m328 -P $(AVRDUDE_PORT) -c arduino -b 115200
AVRDUDE_FLAGS += $(AVRDUDE_NO_VERIFY)
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)
AVRDUDE_FLAGS += -V -F -C /etc/avrdude.conf

HEADERS=adc.h\
	delay.h\
	digio.h\
	eeprom.h\
	eepromParams.h\
	lcd_lib.h\
	pins.h\
	timer.h\
	uart.h\

INCLUDES=$(addprefix $(PREFIX)/include/, $(HEADERS))

OBJS=adc.o\
     delay.o\
     digio.o\
     eeprom.o\
     eepromParams.o\
     lcd_lib.o\
     pins.o\
     timer.o\
     uart.o\

BINS=eeprom_test.elf\
     main.elf\

.phony:	clean all

all:	$(BINS)

#common objects
%.o:	$(PREFIX)/src/%.c $(INCLUDES)
	$(CC) $(CC_OPTS) -c  $<

%.o:	$(PREFIX)/%.c $(INCLUDES)
	$(CC) $(CC_OPTS) -c  $<

%.elf:	%.o $(OBJS)
	$(CC) $(CC_OPTS) -o $@ $< $(OBJS) $(LIBS)

clean:
	rm -rf $(OBJS) $(BINS) *.hex *~ *.o

%.hex:	%.elf
	avr-objcopy -O ihex -R .eeprom $< $@
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:$@:i 
	rm $@
