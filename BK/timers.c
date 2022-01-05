/*
 * timers.c
 * Fclk = 4МГц
 * Created: 02.01.2022 1:10:59
 *  Author: gasra
 */ 

#include "timers.h"

void timer0_init() {
	TCNT0 = 0;
	TCCR0A = 0;				// Mode Normal
	TCCR0B = 0b001;			// Fclk:1, при Fclk=4МГц один отсчет: 0,25 мкс
}

void timer1_init()	{
	TCNT1 = 0;
	TCCR1A = 0;				// Mode Normal
	TCCR1B = 0b010;			// Fclk:8, при Fclk=4МГц один отсчет: 2 мкс
}


void delay_us(uint16_t us) {				// !!! Do not work !!! Нужна более высокая Fclk
	for (uint16_t i = 0; i < us; i++) {
		TCNT0 = 0;	while (TCNT0 < 4);		// Одна итерация - одна микросекунда
	}
}

void delay_ms(uint16_t ms) {
	for (uint16_t i = 0; i < ms; i++) {
		TCNT1 = 0;	while (TCNT1 < 500);	// Одна итерация - одна миллисекунда
	}
}

void delay_s(uint16_t s) {
	for (uint16_t i = 0; i < s; i++)	delay_ms(1000);
}