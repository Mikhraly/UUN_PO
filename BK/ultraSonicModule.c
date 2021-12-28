/*
 * ultraSonicModule.c
 * Created: 09.03.2020 18:05:56
 */ 


#include "ultraSonicModule.h"

void ultrasonicModule_init()	{
	TCCR1A = 0;
	TCCR1B = 0b010;					// Fclk:8, 1 отсчет: 2 мкс
	SET(DDR_USM,TRIGGER);			// Выход импульса, запусающего датчик приближения
	RESET(DDR_USM,ECHO);			// Вход эхо-сигнала
}

uint8_t ultrasonicModule_work {		// Измерение максимум ДО 2 МЕТРОВ!
	SET(PORT_USM,TRIGGER);			// Начало запускающего импульса
	TCNT1 = 0;
	while (TCNT1<10);				// Задержка 20 мкс
	RESET(PORT_USM,TRIGGER);		// Конец запускающего импульса
	while (0==CHECK(PIN_USM,ECHO));	// Пока эхо-сигнала нет - ждать
	TCNT1 = 0;						// Начало измерения длительности эхо-сигнала
	while (1==CHECK(PIN_USM,ECHO));	// Пока эхо-сигнала не закончился - ждать
	return ( (TCNT1*2)/58 + 1 );	// Возвращяет расстояние в сантиметрах. Точность измерения - 1 см из-за деления на 58. +1 добовлено по итогам тестов
}