/*
 * spi_for_MCP3201.c
 * Fclk = 4MHz
 * Created: 16.02.2020 0:03:40
 *
 * � ������ ������������ ������/������� 0 (8-�� ���������)
 * ������������ ����� ���������� ~70��
 */ 

#define	F_CPU	4000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "spi_for_MCP3201.h"


/*******************************************************************************************************************************/
void spi_init() {
	DDR_SPI &= ~(1<<MISO);
	DDR_SPI |= (1<<CLK)|(1<<CS);
	PORT_SPI &= ~(1<<CLK);
	PORT_SPI |= (1<<CS);
}


uint16_t spi_readData() {
	
	uint16_t receiveData = 0;							// ��� �������� ����������� ������ SPI
	
	PORT_SPI &= ~(1<<CLK);								// CLK=0
	PORT_SPI &= ~(1<<CS);								// CS=0
	
	for (uint8_t counterBits = 0; counterBits < 15; counterBits++) {	// ������ ��� ���� �� ����� ����������
		_delay_ms(2);
		receiveData <<= 1;								// ����������� ������� ��� ��� ���������� ���������� ����
		if (PIN_SPI & (1<<MISO)) receiveData |= 0x01;
		_delay_us(500);
		PORT_SPI |= (1<<CLK);
		_delay_ms(2);
		PORT_SPI &= ~(1<<CLK);
	}
	
	PORT_SPI &= ~(1<<CLK);								// CLK=0
	PORT_SPI |= (1<<CS);								// CS=1
	receiveData = receiveData & 0xFFFU;					// �������� ��� (12 ������� ���)
	uint16_t adc8bitsMode = (receiveData>>4) & 0xFFU;	// �������� ��� � 8-�� ������ �������
	return ((125 * adc8bitsMode)/51 - 62);				// ���������� �������� � ��� (10 ������� ���)
}
/*******************************************************************************************************************************/

uint8_t kPaToAtm(uint16_t kPa) {	// �������������� ��� � ���������
	uint8_t count = 0;
	for (uint8_t test_kPa=5; kPa > test_kPa; test_kPa+=10, count++);
	uint8_t high_tetrada = count / 10;
	uint8_t low_tetrada = count % 10;
	return (high_tetrada<<4 | low_tetrada);	
}