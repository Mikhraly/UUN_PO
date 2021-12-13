/*
 * UZD.h
 * Fclk = 4MHz
 * Created: 09.03.2020 18:06:22
 * Author: Lenovo
 * Программа для работы ультразвукового датчика приближения HC-SR04.
 * Точность датчика - 2 см.
 * Суммарная погрешность - 3 см (датчика+измерения)
 * Минимальное время между повторными вызовами функции - 30 мкс.
 * Используется таймер/счетчик 1 (16-ти разрядный).
 * Максимальное время выполнения ~23мс
 */ 


#ifndef UZD_H_
#define UZD_H_


#include <avr/io.h>

#define	DDR_UZD		DDRB
#define PORT_UZD	PORTB
#define PIN_UZD		PINB
#define trigger		4
#define echo		3

#define SET(port,pin)	((port) |= (1<<(pin)))
#define RESET(port,pin)	((port) &= ~(1<<(pin)))
#define CHECK(port,pin)	((port) & (1<<(pin)))

void		UZD_init();			// Функция инициализации датчика приближения
uint8_t		UZD_distance();		// Функция измерения расстояния в сантиметрах. НЕ БОЛЕЕ 200 см!


#endif /* UZD_H_ */