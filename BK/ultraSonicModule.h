/*
 * ultraSonicModule.h
 * Short name - USM
 * Fclk = 4MHz
 * Created: 09.03.2020 18:06:22
 * Author: Lenovo
 * Программа для работы ультразвукового датчика приближения HC-SR04.
 * Точность датчика - 2 см.
 * Суммарная погрешность - 3 см (датчика+измерения)
 * Минимальное время между повторными вызовами функции - 30 мкс.
 * Используется таймер/счетчик 1 (16-ти разрядный).
 * Максимальное время выполнения ~23мс (200 см)
 */ 


#ifndef USM_H_
#define USM_H_


#include <avr/io.h>

#define	DDR_USM		DDRB
#define PORT_USM	PORTB
#define PIN_USM		PINB
#define trigger		4
#define echo		3

#define SET(port,pin)	((port) |= (1<<(pin)))
#define RESET(port,pin)	((port) &= ~(1<<(pin)))
#define CHECK(port,pin)	((port) & (1<<(pin)))

void	ultrasonicModule_init();		// Функция инициализации датчика приближения
uint8_t	ultrasonicModule_work();		// Функция измерения расстояния в сантиметрах. НЕ БОЛЕЕ 200 см!


#endif /* USM_H_*/