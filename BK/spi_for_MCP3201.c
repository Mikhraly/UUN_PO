/*
 * spi_for_MCP3201.c
 * Fclk = 4MHz
 * Created: 16.02.2020 0:03:40
 *
 * � ������ ������������ ������/������� 0 (8-�� ���������)
 * ������������ ����� ���������� ~70��
 */ 

#include "spi_for_MCP3201.h"


/***************************************************************************************************************************************************/
uint16_t spiReadData() {
	
	uint8_t		counterReceiveBits = 0;					// ��� �������� ���������� ����������� ��� SPI
	uint16_t	receiveData = 0;						// ��� �������� ����������� ������ SPI
	
	PORT_SPI &= ~(1<<CLK);								// CLK=0
	PORT_SPI &= ~(1<<CS);								// CS=0
	TCNT0 = 0;
	
	while (counterReceiveBits<15) {						// ������ ��� ���� �� ����� ����������
		while(TCNT0<100);								// �������� 1.6 ��
		receiveData<<=1;								// ����������� ������� ��� ��� ���������� ���������� ����
		if (PIN_SPI & (1<<MISO)) receiveData |= 0x01;
		counterReceiveBits++;
		while (TCNT0 != 128);
		PORT_SPI |= (1<<CLK);
		while (TCNT0 != 0);
		PORT_SPI &= ~(1<<CLK);
	}
	
	PORT_SPI &= ~(1<<CLK);								// CLK=0
	PORT_SPI |= (1<<CS);								// CS=1
	receiveData = receiveData & 0xFFFU;					// �������� ��� (12 ������� ���)
	uint16_t adc8bitsMode = (receiveData>>4) & 0xFFU;	// �������� ��� � 8-�� ������ �������
	return ((125 * adc8bitsMode)/51 - 62);				// ���������� �������� � ��� (10 ������� ���)
}
/***************************************************************************************************************************************************/

uint8_t kPaToAtm(uint16_t kPa) {	// �������������� ��� � ���������
	uint8_t count = 0;
	for (uint8_t test_kPa=5; kPa > test_kPa; test_kPa+=10, count++);
	uint8_t high_tetrada = count / 10;
	uint8_t low_tetrada = count % 10;
	return (high_tetrada<<4 | low_tetrada);	
}



void timer0_init() {
	TCNT0 = 0;
	TCCR0A = 0;
	TCCR0B |= 1<<CS01|1<<CS00;		// Fclk:64	(��� Fclk=4��� ���� ������ ������� �� 16���)
}

void spi_init() {
	DDR_SPI &= ~(1<<MISO);
	DDR_SPI |= (1<<CLK)|(1<<CS);
	PORT_SPI &= ~(1<<CLK);
	PORT_SPI |= (1<<CS);
	timer0_init();
}
