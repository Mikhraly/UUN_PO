/*
 * spi_for_MCP3201.h
 * Fclk = 4MHz
 * Created: 16.02.2020 0:06:32
 * Author: Lenovo
 * ��������� ��� ������ �������� 12-�� ���������� ��� MCP3201.
 * ������������ ����������� ���������� SPI-����������.
 * ������������ ������/������� 0.
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

// ����� �������� ���� � ��� (10 ������� ���)
uint16_t spiReadData();

/* ���������� ���������, ���
 * ������� ������� - ����� �����,
 * ������� ������� - ������� �����
 */								
uint8_t convert_kPaToAtm(uint16_t kPa);


#endif /* SPI_FOR_MCP3201_H_ */