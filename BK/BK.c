/*
 * BK.c
 * Fclk=4MHz
 * Created: 14.12.2019 19:32:54
 */

#define	F_CPU	4000000UL									// Тактовая частота микроконтроллера

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/crc16.h>
#include <util/delay.h>
#include "timers.h"
#include "uart.h"
#include "spi_for_MCP3201.h"
#include "ultraSonicModule.h"


volatile uint8_t	num = 1;								// Номер принятого байта по UART
volatile uint8_t	rec_byte[4];							// Массив принимаемых байт. [0] - не используется
volatile uint8_t	tran_byte[6];							// Массив отправляемых байт. [0] - не используется
volatile uint8_t	recMessageOK;							// Флаг завершения приема сообщения
volatile uint16_t	counterConnectionNOK = 0;


ISR (USART_RX_vect) {										// Функция приема байта по UART через прерывание
	rec_byte[num] = UDR;									// Считать данные
	if(num==1 && rec_byte[1] != 0x7E) return;
	if(num == 3) {
		num = 1;
		uint8_t crc8 = 0xFF;
		for (uint8_t i=1; i<=2; i++) {
			crc8 = _crc8_ccitt_update(crc8, rec_byte[i]);
		}
		if(crc8 == rec_byte[3]) {
			recMessageOK = 1;
			UCSRB &= ~(1<<RXCIE);	// Выкл прерывание по приему
		}
		else return;				// Если КС не совпала, выйти и начать сначала
	}
	else num++;
}

ISR (TIMER0_COMPA_vect) {
	counterConnectionNOK++;
	TCNT0 = 0;
}

uint8_t distanceToProcent(const uint8_t distance, const uint8_t sensorHigh);
void pumpON();
void pumpOFF();


int main(void)
{
	asm("cli");							// Выкл глобальные прерывания
	asm("wdr");							// Сбросить WDT (обнулить значение)
	MCUSR &= ~(1<<WDRF);				// Сбросить флаг сброса WDT
	WDTCR |= (1<<WDCE) | (1<<WDE);		// Вкл WDT в режиме системного сброса
	WDTCR |= (1<<WDP3) | (1<<WDP0);		// Время до сброса - 8 секунд
	
	
	uint8_t	waterLevel = 0;			// Переменная для хранения уровня воды в сантиметрах
	uint8_t	waterPressure = 0;		// Переменная для хранения давления воды в кПа (максимум 10 бит)
	uint8_t	pumpStatus = 0;			// Переменная для хранения состояния насоса (вкл/выкл)
	
	timer0_init();
	timer1_init();
	uart_init();
	spi_init();
	ultrasonicModule_init();
	
	// Инициализация портов ввода/вывода
	DDRD &= ~(1<<5);		// Вход для считывания состояния насоса вкл/выкл
	DDRD |=  (1<<6);		// Выход команды ВКЛ
	DDRB |=  (1<<0);		// Выход команды ВЫКЛ
	DDRB |=  (1<<1);		// Выход команды прием/передача на MAX485
	PORTD |=  (1<<5);		// Внутренний подтягивающий резистор
	PORTD &= ~(1<<6);		// Нет команды ВКЛ
	PORTB &= ~(1<<0);		// Нет команды ВЫКЛ
	PORTB &= ~(1<<1);		// MAX485 на прием
	
	pumpOFF();
	
	asm("sei");				// Вкл глобальные прерывания для работы приемника UART
	
    while (1) 
    {
		/************************************************************************/
		/*                    Обработка сообщения из UART                       */
		/************************************************************************/
		if (recMessageOK) {					// Если прием сообщения завершен успешно
			recMessageOK = 0;				// Сбросить флаг успешного приема
			UCSRB &= ~(1<<RXCIE);			// Выкл прерывание UART по приему
			PORTB |= 1<<1;					// MAX485 на передачу
			_delay_us(200);
			
			if (rec_byte[2] & 1<<0) pumpON();
			if (rec_byte[2] & 1<<1) pumpOFF();
				
			if (rec_byte[2] & 1<<2) {								// Запрос уровня воды
				waterLevel = distanceToProcent(ultrasonicModule_work(), 20);		// Замер уровня воды (в процентах)
			}
			if (rec_byte[2] & 1<<3) {								// Запрос давления
				waterPressure = kPaToAtm(spi_readData());			// Замер давления воды (в атм. bbbb,bbbb)
			}
			if (rec_byte[2] & 1<<4) {								// Запрос статуса
				pumpStatus = (~PIND & 1<<5)?	1 : 0;				// Считать состояние насоса
			}
			
			// Отправка данных в UART
			tran_byte[1] = 0x7E;			uart_transmitByte(tran_byte[1]);
			tran_byte[2] = pumpStatus<<7;	uart_transmitByte(tran_byte[2]);
			tran_byte[3] = waterPressure;	uart_transmitByte(tran_byte[3]);
			tran_byte[4] = waterLevel;		uart_transmitByte(tran_byte[4]);
			// Подсчет контрольной суммы
			uint8_t crc8 = 0xFF;
			for (uint8_t i=1; i<=4; i++) {
				crc8 = _crc8_ccitt_update(crc8, tran_byte[i]);
			}
			tran_byte[5] = crc8;			uart_transmitByte(tran_byte[5]);
			while ( !(UCSRA & (1<<TXC)) );		// Ждем завершения передачи
			UCSRA |= 1<<TXC;					// Сбрасываем флаг завершения передачи
			
			_delay_us(200);
			PORTB &= ~(1<<1);			// MAX485 на прием
			_delay_us(200);
			UCSRB |= (1<<RXCIE);		// Вкл прерывание по приему
			
			counterConnectionNOK = 0;
		}
		//////////////////////////////////////////////////////////////////////////
		
		if (counterConnectionNOK > 800) {	// 8 sec
			counterConnectionNOK = 0;
			if (~PIND & 1<<5) pumpOFF();
		}
		
		asm("wdr");		// Сбросить WDT (обнулить значение)
    }
}




/* Измерение уровня воды в емкости (лежачий цилиндр с окружностью диаметром 86см)
 * Возвращает уровень в процентах
 * distance - расстояние до поверхности измеренное датчиком
 * sensorHigh - высота датчика над 100%-м уровнем.
 */ 
uint8_t distanceToProcent(const uint8_t distance, const uint8_t sensorHeight) {
	if (distance < sensorHeight) return 100;
	if (distance < sensorHeight+8) return 95;
	if (distance < sensorHeight+13) return 90;
	if (distance < sensorHeight+17) return 85;
	if (distance < sensorHeight+21) return 80;
	if (distance < sensorHeight+25) return 75;
	if (distance < sensorHeight+29) return 70;
	if (distance < sensorHeight+32) return 65;
	if (distance < sensorHeight+36) return 60;
	if (distance < sensorHeight+39) return 55;
	if (distance < sensorHeight+43) return 50;
	if (distance < sensorHeight+46) return 45;
	if (distance < sensorHeight+49) return 40;
	if (distance < sensorHeight+53) return 35;
	if (distance < sensorHeight+56) return 30;
	if (distance < sensorHeight+60) return 25;
	if (distance < sensorHeight+64) return 20;
	if (distance < sensorHeight+68) return 15;
	if (distance < sensorHeight+72) return 10;
	if (distance < sensorHeight+77) return 5;
	return 0;
}

void pumpON() {
	PORTD |= 1<<6;				// Начало импульса на включение
	do { _delay_ms(1000);
	} while (PIND & 1<<5);		// Повторять задержку пока не включится
	PORTD &= ~(1<<6);			// Конец импульса на включение
}

void pumpOFF() {
	PORTB |= 1<<0;				// Начало импульса на выключение
	do { _delay_ms(1000);
	} while (~PIND & 1<<5);		// Повторять задержку пока не выключится
	PORTB &= ~(1<<0);			// Конец импульса на выключение
}