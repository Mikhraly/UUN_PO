/*
 * spi_for_MCP3201.h
 * Fclk = 4MHz
 * Created: 16.02.2020 0:06:32
 * Author: Lenovo
 * ��������� ��� ������ �������� 12-�� ���������� ��� MCP3201,
 * � �������� ��������� ������ �������� HK3022.
 * ������������ ����������� ���������� SPI-����������.
 * ������������ ������/������� 0.
 */ 


#ifndef SPI_FOR_MCP3201_H_
#define SPI_FOR_MCP3201_H_


#include <avr/io.h>
#include "timers.h"

#define		DDR_SPI		DDRD
#define		PIN_SPI		PIND
#define		PORT_SPI	PORTD
#define		CLK			2
#define		MISO		3
#define		CS			4

void spi_init();
uint16_t spi_readData();	// ����� �������� ���� � ��� (10 ������� ���)

/* ���������� ���������, ���
 * ������� ������� - ����� �����,
 * ������� ������� - ������� �����
 */								
uint8_t kPaToAtm(uint16_t kPa);


#endif /* SPI_FOR_MCP3201_H_ */