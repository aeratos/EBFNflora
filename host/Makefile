PREFIX = .
CC = gcc
CC_OPTS = -Wall -g --std=gnu99 -I$(PREFIX)/../include


OBJS = packet_handler.o\
	serial_linux.o\


HEADERS = packet_header.h\
	  packet_operations.h\
	  packet_handler.h\


INCLUDES = $(addprefix $(PREFIX)/../include/, $(HEADERS))


BINS = sendParams.bin\


.phony:	clean all


all:	$(BINS)


%.o:	$(PREFIX)/../src/%.c $(INCLUDES)
	$(CC) $(CC_OPTS) -c  $<

%.o:	$(PREFIX)/%.c $(INCLUDES)
	$(CC) $(CC_OPTS) -c  $<

%.bin:  %.o $(OBJS) $(INCLUDES)
	$(CC) $(CC_OPTS) -o $@ $< $(OBJS) $(LIBS)

clean:
	rm -rf $(OBJS) $(BINS) *~
