/*
 * timers.c
 * Fclk=4���
 * Created: 02.01.2022 1:10:59
 *  Author: gasra
 */ 

#include "timers.h"

void timer0_init() {
	TCCR0A = 0;				// Mode Normal
	TCCR0B = 0b001;			// Fclk:1, ��� Fclk=4��� ���� ������: 0,25 ���
}

void timer1_init()	{
	TCCR1A = 0;				// Mode Normal
	TCCR1B = 0b010;			// Fclk:8, ��� Fclk=4��� ���� ������: 2 ���
}


void delay_us(uint8_t us) {
	for (uint8_t i = 0; i < us; i++) {
	TCNT0 = 0;	while (TCNT0 < 4);		// ���� �������� - ���� ������������
	}
}

void delay_ms(uint8_t ms) {
	for (uint8_t i = 0; i < ms; i++) {
	TCNT0 = 0;	while (TCNT0 < 500);	// ���� �������� - ���� ������������
	}
}

void delay_s(uint8_t s) {
	for (uint8_t i = 0; i < s; i++)	delay_ms(1000);
}