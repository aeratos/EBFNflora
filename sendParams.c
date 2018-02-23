#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>

#define STR_LENGTH 16

speed_t baud = B115200; /* baud rate */

int main(){
	char byte;
	char s[STR_LENGTH];
	
	int fd = open("/dev/ttyACM0", O_RDWR);
	if(fd < 0){
		perror("error");
		exit(-1);
	}
	
	/* set the other settings */
	struct termios settings;
	tcgetattr(fd, &settings);

	cfsetospeed(&settings, baud); /* baud rate */
	settings.c_cflag &= ~PARENB; /* no parity */
	settings.c_cflag &= ~CSTOPB; /* 1 stop bit */
	settings.c_cflag &= ~CSIZE;
	settings.c_cflag |= CS8 | CLOCAL; /* 8 bits */
	settings.c_lflag = ICANON; /* canonical mode */
	settings.c_oflag &= ~OPOST; /* raw output */

	tcsetattr(fd, TCSANOW, &settings); /* apply the settings */
	tcflush(fd, TCOFLUSH);

	printf("Waiting for messages...\n");

	while(1){
		/* Reading */
		while(1){
			ssize_t n = read(fd, &byte, 1);
			if(n <= 0){
				perror("error");
				exit(-1);
			}
			printf("%c", byte);
			if(byte == '\0' || byte == '\n' || byte == '\r')
				break;
		}
	
		/* Writing */
		fgets(s, STR_LENGTH, stdin);
		if((strlen(s) > 0) && (s[strlen(s) - 1] == '\n'))
			s[strlen(s) - 1] = '\0';
			
		int i;
		int l = strlen(s);
		for (i=0; i<l; i++) {
			if(write(fd, s+i, 1) < 1){
				perror("error");
				exit(-1);
			}
		}
		write(fd, "\0", 1);
	
	}

	return 0;
}
