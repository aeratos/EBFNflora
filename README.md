# EBFNflora

![GitHub Logo](circuit.jpg)

All'avvio vengono inizializzati i registri per la comunicazione seriale (UART), l'uso dei timer (TIMER1), i pin digitali, l'LCD (usata [libreria esterna](http://winavr.scienceprog.com/example-avr-projects/avr-gcc-4-bit-and-8-bit-lcd-library.html)), l'interfaccia ADC (scritta appositamente in [adc.c](src/adc.c)) e gli external interrupt (INT0, INT1). Dopo aver attivato gli interrupt globali

```c
EIMSK = (1<<INT1)|(1<<INT0); //Turn ON INT0 and INT1
EICRA = (1<<ISC11)|(1<<ISC10)|(1<<ISC01)|(1<<ISC00);
sei(); //enable interrupts globally
```

si entra in un loop in cui si richiede ripetutamente all'utente se intende configurare i parametri di misurazione in EEPROM:

```c
while(waitingForOutput){
	LCDclr();
	printOnLCD("Setup sensors?");
	LCDGotoXY(0,1);
	printOnLCD("[Y/n]?");
	delayMs(300);
}
```

Se viene premuto il bottone collegato al pin 2 dell'Arduino Uno (INT0) la variabile globale reset è settata a 1, altrimenti se viene premuto quello relativo al pin 3 (INT1), reset è settato a 0. In entrambi i casi il flag waitingForOutput è settato a 0 causando così l'uscita dal ciclo.

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

Nel primo caso (reset = 1) l'MCU interagisce con l'utente per ottenere i nuovi parametri di configurazione e salvarli in EEPROM, ognuno in blocchi da 16 byte come di seguito riportato. Sono state scritte funzioni apposite per lettura/scrittura da/su EEPROM definite in [eepromParams.c](src/eepromParams.c)

![GitHub Logo](EBFNflora_buffer.PNG)

Nel secondo caso (reset = 0) i valori già presenti in EEPROM - quelli dell'ultima inizializzazione - sono salvati subito in variabili globali.

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

Il valore in timer è usato per richiamare ogni timer millisecondi la funzione timerFn che si occupa di effettuare il sensing dell'ambiente.

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

**N.B**: il TIMER1 con un prescaler di 128 è in grado di supportare una durata massima di 4195 ms: un valore maggiore causa un overflow portando così il timeout 0 ms.

## Sensori

### Fotoresistore

Il sensore di luce varia la propria resistenza a seconda della luce che cattura. Nel progetto è stato collegato al pin ADC5.

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

La lettura del pin analogico restituisce un valore tra 0 e 1023. Se non ricade nell'intervallo `[minLinght,maxLight]` viene invocata la funzione error_blink() e stampato un 
messaggio di errore.

### Sensore qualità dell'aria MQ 135

Il sensore di inquinamento (MQ 135) indica la quantità di gas presente nell'ambiente circostante.
Esso è composto da un esoscheletro di acciaio che circonda un elemento "recettore" il quale si ionizza al contatto con i gas variando la corrente che fa passare.

Nel progetto esso è collegato al pin ADC3.

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

La lettura del pin analogico restituisce un valore tra 0 e 1023. Se è inferiore a maxPoll viene invocata la funzione error_blink() e stampato un messaggio di errore. Alcuni valori di riferimento:

- condizioni normali: circa 100-150
- alcol: circa 700
- polveri sottili: circa 750

### Termistore

Il termistore varia la sua resistenza in base alla temperatura grazie al materiale di cui è composto: ossido di metallo o ceramica. Nel progetto esso è collegato al pin ADC0.

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
* la variabile V0 memorizza il valore di ritorno di adc_read();
* tramite una formula basata sull'equazione di Steinhart-Hart `R2 = R1 * (1023.0 / (float)Vo - 1.0)` si ricava la resistenza attuale del termistore;
* la formula `T = (1.0 / (C1 + C2*logR2 + C3*logR2*logR2*logR2))` converte il valore della resistenza R2 in gradi Kelvin;
* `Tc = T - 273.15` lo converte in °C.

Se la temperatura attuale non è nell'intervallo `[minTemp,maxTemp]` viene invocata la funzione error_blink() e viene stampato un messaggio di errore.

