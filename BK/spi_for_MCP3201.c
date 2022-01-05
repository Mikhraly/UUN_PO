/*
 * spi_for_MCP3201.c
 * Fclk = 4MHz
 * Created: 16.02.2020 0:03:40
 *
 * В работе задействован таймер/счетчик 0 (8-ми разрядный)
 * Максимальное время выполнения ~70мс
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
	
	uint16_t receiveData = 0;							// Для хранения считываемых данных SPI
	
	PORT_SPI &= ~(1<<CLK);								// CLK=0
	PORT_SPI &= ~(1<<CS);								// CS=0
	
	for (uint8_t counterBits = 0; counterBits < 15; counterBits++) {	// Первые три бита не несут информации
		_delay_ms(2);
		receiveData <<= 1;								// Освобождаем младший бит для считывания очередного бита
		if (PIN_SPI & (1<<MISO)) receiveData |= 0x01;
		_delay_us(500);
		PORT_SPI |= (1<<CLK);
		_delay_ms(2);
		PORT_SPI &= ~(1<<CLK);
	}
	
	PORT_SPI &= ~(1<<CLK);								// CLK=0
	PORT_SPI |= (1<<CS);								// CS=1
	receiveData = receiveData & 0xFFFU;					// Значение АЦП (12 младших бит)
	uint16_t adc8bitsMode = (receiveData>>4) & 0xFFU;	// Значение АЦП в 8-ми битном формате
	return ((125 * adc8bitsMode)/51 - 62);				// Возвращяет давление в кПа (10 младших бит)
}
/*******************************************************************************************************************************/

uint8_t kPaToAtm(uint16_t kPa) {	// преобразование кПа в атмосферы
	uint8_t count = 0;
	for (uint8_t test_kPa=5; kPa > test_kPa; test_kPa+=10, count++);
	uint8_t high_tetrada = count / 10;
	uint8_t low_tetrada = count % 10;
	return (high_tetrada<<4 | low_tetrada);	
}