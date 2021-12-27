/*
 * spi_for_MCP3201.h
 * Fclk = 4MHz
 * Created: 16.02.2020 0:06:32
 * Author: Lenovo
 * Программа для работы внешнего 12-ти разрядного АЦП MCP3201.
 * Используется программная реализация SPI-интерфейса.
 * Используется таймер/счетчик 0.
 */ 


#ifndef SPI_FOR_MCP3201_H_
#define SPI_FOR_MCP3201_H_


#include <avr/io.h>

#define		DDR_SPI		DDRD
#define		PIN_SPI		PIND
#define		PORT_SPI	PORTD
#define		CLK			2
#define		MISO		3
#define		CS			4

void timer0_init();
void spi_init();

// Замер давления воды в кПа (10 младших бит)
uint16_t spiReadData();

/* Возвращает атмосферы, где
 * старшая тетрада - целая часть,
 * младшая тетрада - десятая часть
 */								
uint8_t convert_kPaToAtm(uint16_t kPa);


#endif /* SPI_FOR_MCP3201_H_ */