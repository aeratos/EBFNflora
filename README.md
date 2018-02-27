# EBFNflora

*EBFNflora* è il nome di una applicazione embedded per Arduino Uno (ATmega328) inizialmente pensata per monitorare l'ambiente in cui si trova una pianta. Il microcontrollore monitora ad un intervallo regolare - configurabile dall'utente - la quantità di luce, la qualità dell'aria e la temperatura esterna. In caso di anomalie viene mostrato sul display un apposito messaggio di warning accompagnato dal blink di un LED e dal beep di un active buzzer. I valori di soglia per ogni sensore sono anch'essi configurabili.

![GitHub Logo](circuit.jpg)

All'avvio vengono inizializzati i registri per la comunicazione seriale (UART), l'uso dei timer (TIMER1), i pin digitali, l'LCD (usata [libreria esterna](http://winavr.scienceprog.com/example-avr-projects/avr-gcc-4-bit-and-8-bit-lcd-library.html)), l'interfaccia ADC (definita appositamente in [adc.c](src/adc.c)) e gli external interrupt (INT0, INT1).

```c
EIMSK = (1<<INT1)|(1<<INT0); //Turn ON INT0 and INT1
EICRA = (1<<ISC11)|(1<<ISC10)|(1<<ISC01)|(1<<ISC00);
```

L'mcu legge da EEPROM i parametri necessari per misurare la temperatura, quelli dell'ultima inizializzazione. A questo punto, dopo un messaggio di benvenuto sul display, si entra in un loop in cui si chiede ripetutamente all'utente (output sul display) se intende configurare i parametri di misurazione in EEPROM e nel frattempo si visualizza sull'LCD la temperatura corrente.

```c
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
```

Se viene premuto il bottone collegato alla porta PD2 (INT0) la variabile globale `reset` è settata a 1, altrimenti se viene premuto quello relativo alla porta PD3 (INT1), `reset` è settato a 0. In entrambi i casi il flag `waitingForOutput` è settato a 0 causando così l'uscita dal ciclo while.

```c
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
```

Nel primo caso (`reset = 1`) l'MCU interagisce con l'utente tramite seriale per ottenere i nuovi parametri di configurazione e salvarli in EEPROM, ognuno in blocchi da 16 byte come di seguito riportato. Sono state scritte funzioni apposite per lettura/scrittura da/su EEPROM definite in [eepromParams.c](src/eepromParams.c)

![GitHub Logo](EBFNflora_buffer.PNG)

* `uint16_t timer`: intervallo in ms tra due successivi controlli dei sensori
* `uint16_t minLight`
* `uint16_t maxLight`
* `uint16_t minTemp`
* `uint16_t maxTemp`
* `uint16_t maxPoll`: massimo valore della qualità dell'aria tollerato, consigliabile 150-170
* `float R1`: resistenza per il calcolo della temperatura, nel circuito 10000
* `float C1, C2, C3`: costanti, vedere [Termistore](#Termistore)

**N.B**: il programma [eeprom_test.c](eeprom_test.c) è stato scritto per leggere i valori correnti in EEPROM.

Nel secondo caso (`reset = 0`) i valori già presenti in EEPROM - quelli dell'ultimo setup - sono salvati subito nelle variabili globali.

```c
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
```

Il valore in `timer` è usato per richiamare ogni "timer" ms la funzione `timerFn()` che si occupa di effettuare il sensing dell'ambiente.

```c
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
```

**N.B**: il TIMER1 è stato settato in [timer.c](src/timer.c) con un prescaler di 1024 ed è quindi in grado di supportare una durata massima di 4195 ms. Un valore maggiore causa un overflow e azzera così il timeout.

Il microcontrollore entra infine in un loop vuoto ed eseguirà la funzione `timerFn()` ad ogni compare interrupt.

## Comunicazione seriale

Il baudrate selezionato è 115200. Il formato dei frame adottato in [uart.c](src/uart.c) prevede 1 start bit, 8 bits data (`UCSR0C = _BV(UCSZ01) | _BV(UCSZ00)`), nessun bit di parità (nel registro UCSR0C i bit UPM01=0 e UPM00=0), 1 stop bit (nel registro UCSR0C il bit USBS0=0). Il programma [sendParams.c](sendParams.c) scritto per comunicare via terminale e configurare così i parametri in EEPROM utilizza la stessa configurazione per i pacchetti.

```c
speed_t baud = B115200; /* baud rate */

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
```
Il programma riconosce la fine di un messaggio grazie a un carattere speciale (`'\0' oppure '\n'`), legge da `stdin` la stringa digitata dall'utente e la invia al microcontrollore dopo aver aggiunto il terminatore `\0`. Il ciclo è ripetuto 7 volte, cioé per ogni parametro configurabile in EEPROM.

Dall'altro lato l'MCU salva la stringa ricevuta nella EEPROM, e.g. la variabile timer:

```c
printString("timer in ms?\n");
readString(rx_message, BUFFER_SIZE);
set_EEPROM_timer(rx_message);
```

**N.B**: `void readString(char* dest, int size)` è una funzione definita in [main.c](main.c) per salvare in un buffer i caratteri ricevuti.

## Sensori

### Fotoresistore

Il sensore di luce varia la propria resistenza a seconda della luce che cattura. Nel progetto è collegato al pin ADC5.

```c
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
```

La lettura del pin analogico restituisce un valore tra 0 e 1023. Se non ricade nell'intervallo `[minLinght,maxLight]` viene invocata la funzione `error_blink()` e stampato un 
messaggio di errore.

### Sensore qualità dell'aria MQ 135

Il sensore di inquinamento (MQ 135) indica la quantità di gas presente nell'ambiente circostante.
Esso è composto da un esoscheletro di acciaio che circonda un elemento "recettore" il quale si ionizza al contatto con i gas variando la corrente che fa passare. Nel progetto è collegato al pin ADC3.

```c
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
```

La lettura del pin analogico restituisce un valore tra 0 e 1023. Se è inferiore a maxPoll viene invocata la funzione `error_blink()` e stampato un messaggio di errore. Alcuni valori di riferimento:

- condizioni normali: circa 100-150
- alcol: circa 700
- polveri sottili: circa 750

### Termistore

Il termistore varia la sua resistenza in base alla temperatura grazie al materiale di cui è composto: ossido di metallo o ceramica. Nel progetto è collegato al pin ADC0.

```c
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
```

La variabile Tc indica la temperatura attuale.
* la variabile V0 memorizza il valore di ritorno di `adc_read()`;
* tramite una formula basata sull'equazione di Steinhart-Hart `R2 = R1 * (1023.0 / (float)Vo - 1.0)` si ricava la resistenza attuale del termistore;
* la formula `T = (1.0 / (C1 + C2*logR2 + C3*logR2*logR2*logR2))` converte il valore della resistenza R2 in gradi Kelvin;
* `Tc = T - 273.15` lo converte in °C.

Se la temperatura attuale non è nell'intervallo `[minTemp,maxTemp]` viene invocata la funzione `error_blink()` e viene stampato un messaggio di errore.

