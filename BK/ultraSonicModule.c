/*
 * ultraSonicModule.c
 * Created: 09.03.2020 18:05:56
 */ 

#define	F_CPU	4000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "ultraSonicModule.h"

void ultrasonicModule_init() {
	SET(DDR_USM,TRIGGER);					// Выход импульса, запусающего датчик приближения
	RESET(DDR_USM,ECHO);					// Вход эхо-сигнала
}

uint8_t ultrasonicModule_work() {			// Измерение максимум ДО 2 МЕТРОВ!
	SET(PORT_USM,TRIGGER);					// Начало запускающего импульса
	_delay_us(20);
	RESET(PORT_USM,TRIGGER);				// Конец запускающего импульса
	while ( !(CHECK(PIN_USM,ECHO)) );		// Пока эхо-сигнала нет - ждать
	TCNT1 = 0;								// Начало измерения длительности эхо-сигнала
	while ( CHECK(PIN_USM,ECHO) );			// Пока эхо-сигнала не закончился - ждать
	return ( (uint8_t)((TCNT1*2)/58 + 1) );	// Возвращяет расстояние в сантиметрах. Точность измерения - 1 см из-за деления на 58.
											// +1 добовлено по итогам тестов
}