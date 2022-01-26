/*
 * timers.c
 * Fclk = 4МГц
 * Created: 23.01.2022 16:00:38
 *  Author: gasra
 */ 

#include "timers.h"
/*
void timer0_init() {
	TCNT0 = 0;
	TIMSK |= 1<<OCIE0;		// Разрешено прерывание по совпадению
	TCCR0 = 0;				// Mode Normal
	TCCR0B = 0b001;			// Fclk:8, при Fclk=4МГц один отсчет: 2 мкс
}
*/

void timer1_init() {
	TCNT1 = 0;
	TCCR1A = 0;				// Mode Normal
	TCCR1B = 0b100;			// Fclk:256, при Fclk=4МГц один отсчет: 64 мкс
	TIMSK |= 1<<OCIE1A;		// Разрешено прерывание по совпадению TIMER1_COMPA_vect
	OCR1A = 15625;			// Прерывание каждую секунду
}
