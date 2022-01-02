/*
 * timers.h
 *
 * Created: 02.01.2022 1:12:13
 *  Author: gasra
 */ 


#ifndef TIMERS_H_
#define TIMERS_H_


#include <avr/io.h>

void timer0_init();
void timer1_init();

void delay_us(uint8_t us);
void delay_ms(uint8_t ms);


#endif /* TIMERS_H_ */