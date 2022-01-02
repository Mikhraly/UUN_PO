/*
 * timers.c
 * Fclk=4ћ√ц
 * Created: 02.01.2022 1:10:59
 *  Author: gasra
 */ 

#include "timers.h"

void timer0_init() {
	TCCR0A = 0;				// Mode Normal
	TCCR0B = 0b001;			// Fclk:1, при Fclk=4ћ√ц один отсчет: 0,25 мкс
}

void timer1_init()	{
	TCCR1A = 0;				// Mode Normal
	TCCR1B = 0b010;			// Fclk:8, при Fclk=4ћ√ц один отсчет: 2 мкс
}


void delay_us(uint8_t us) {
	for (uint8_t i = 0; i < us; i++) {
	TCNT0 = 0;	while (TCNT0 < 4);		// ќдна итераци€ - одна микросекунда
	}
}

void delay_ms(uint8_t ms) {
	for (uint8_t i = 0; i < ms; i++) {
	TCNT0 = 0;	while (TCNT0 < 500);	// ќдна итераци€ - одна миллисекунда
	}
}

void delay_s(uint8_t s) {
	for (uint8_t i = 0; i < s; i++)	delay_ms(1000);
}