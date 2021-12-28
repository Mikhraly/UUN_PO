/*
 * spi_for_MCP3201.c
 * Fclk = 4MHz
 * Created: 16.02.2020 0:03:40
 *
 * В работе задействован таймер/счетчик 0 (8-ми разрядный)
 * Максимальное время выполнения ~70мс
 */ 

#include "spi_for_MCP3201.h"


/***************************************************************************************************************************************************/
uint16_t spiReadData() {
	
	uint8_t		counterReceiveBits = 0;					// Для подсчета количества принимаемых бит SPI
	uint16_t	receiveData = 0;						// Для хранения считываемых данных SPI
	
	PORT_SPI &= ~(1<<CLK);								// CLK=0
	PORT_SPI &= ~(1<<CS);								// CS=0
	TCNT0 = 0;
	
	while (counterReceiveBits<15) {						// Первые три бита не несут информации
		while(TCNT0<100);								// Задержка 1.6 мс
		receiveData<<=1;								// Освобождаем младший бит для считывания очередного бита
		if (PIN_SPI & (1<<MISO)) receiveData |= 0x01;
		counterReceiveBits++;
		while (TCNT0 != 128);
		PORT_SPI |= (1<<CLK);
		while (TCNT0 != 0);
		PORT_SPI &= ~(1<<CLK);
	}
	
	PORT_SPI &= ~(1<<CLK);								// CLK=0
	PORT_SPI |= (1<<CS);								// CS=1
	receiveData = receiveData & 0xFFFU;					// Значение АЦП (12 младших бит)
	uint16_t adc8bitsMode = (receiveData>>4) & 0xFFU;	// Значение АЦП в 8-ми битном формате
	return ((125 * adc8bitsMode)/51 - 62);				// Возвращяет давление в кПа (10 младших бит)
}
/***************************************************************************************************************************************************/

uint8_t kPaToAtm(uint16_t kPa) {	// преобразование кПа в атмосферы
	uint8_t count = 0;
	for (uint8_t test_kPa=5; kPa > test_kPa; test_kPa+=10, count++);
	uint8_t high_tetrada = count / 10;
	uint8_t low_tetrada = count % 10;
	return (high_tetrada<<4 | low_tetrada);	
}



void timer0_init() {
	TCNT0 = 0;
	TCCR0A = 0;
	TCCR0B |= 1<<CS01|1<<CS00;		// Fclk:64	(при Fclk=4МГц один отсчет таймера за 16мкс)
}

void spi_init() {
	DDR_SPI &= ~(1<<MISO);
	DDR_SPI |= (1<<CLK)|(1<<CS);
	PORT_SPI &= ~(1<<CLK);
	PORT_SPI |= (1<<CS);
	timer0_init();
}
