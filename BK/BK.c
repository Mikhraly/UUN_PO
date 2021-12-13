/*
 * BK.c
 * Fclk=4MHz
 * Created: 14.12.2019 19:32:54
 */
#define	F_CPU	4000000UL									// Тактовая частота микроконтроллера

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "spi_for_MCP3201.h"
#include "UZD.h"

#define	BAUD	9600UL										// Скорость обмена по UART
#define SPEED	(F_CPU/(BAUD*16)-1)							// Константа для записи в регистр скорости передачи UART

volatile uint8_t	num = 1;								// Номер принятого байта по UART
volatile uint8_t	rec_byte[4];							// Массив принимаемых байт. [0] - не используется
volatile uint8_t	recMessageOK;							// Флаг завершения приема сообщения


ISR (USART0_RX_vect) {										// Функция приема байта по UART через прерывание
	rec_byte[num] = UDR;									// Считать данные
	if(num==1 && rec_byte[1] != 0x7E) return;
	num++;
	if (num == 4) {
		num = 1;
		if(rec_byte[1] + rec_byte[2] != rec_byte[3]) return;// Если КС не совпала, выйти и начать сначала
		UCSRB &= ~(1<<RXCIE);								// Выкл прерывание по приему
		recMessageOK = 1;
	}
}

void	uart_init();
uint8_t	uart_receive_byte();
void	uart_transmit_byte(uint8_t);
uint8_t waterProcent(const uint8_t distance, const uint8_t sensorHigh);


int main(void)
{
	asm("cli");							// Выкл глобальные прерывания
	asm("wdr");							// Сбросить WDT (обнулить значение)
	MCUSR &= ~(1<<WDRF);				// Сбросить флаг сброса WDT
	WDTCR |= (1<<WDCE) | (1<<WDE);		// Вкл WDT в режиме системного сброса
	WDTCR = (1<<WDE) | (1<<WDP3);		// Время до сброса - 4 секунды
	
	
	uint8_t		waterLevel = 0;			// Переменная для хранения уровня воды в сантиметрах
	uint8_t		waterPressure = 0;		// Переменная для хранения давления воды в кПа (максимум 10 бит)
	uint8_t		pumpStatus = 0;			// Переменная для хранения состояния насоса (вкл/выкл)
	
	uart_init();
	SPI_init();
	UZD_init();
	
	DDRD &= ~(1<<5);		// Вход для считывания состояния насоса вкл/выкл
	DDRD |=  (1<<6);		// Выход команды ВКЛ
	DDRB |=  (1<<0);		// Выход команды ВЫКЛ
	DDRB |=  (1<<1);		// Выход команды прием/передача на MAX485
	PORTD |=  (1<<5);		// Внутренний подтягивающий резистор
	PORTD &= ~(1<<6);		// Нет команды ВКЛ
	PORTB &= ~(1<<0);		// Нет команды ВЫКЛ
	PORTB &= ~(1<<1);		// MAX485 на прием
	
	asm("sei");	// Вкл глобальные прерывания для работы приемника UART
	
	// Выключение насоса
	PORTB |= 1<<0;			// Начало импульса на выключение
	while (~PIND & 1<<5);	// Ждать выключения насоса
	PORTB &= ~(1<<0);		// Конец импульса на выключение
	
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
			
			if (rec_byte[2] & 1<<0) {		// Запрос на ВКЛ
				PORTD |= 1<<6;				// Начало импульса на включение
				while (PIND & 1<<5);		// Ждать включения насоса
				_delay_ms(500);
				PORTD &= ~(1<<6);			// Конец импульса на включение
			}
			if (rec_byte[2] & 1<<1) {		// Запрос на ВЫКЛ
				PORTB |= 1<<0;				// Начало импульса на выключение
				while (~PIND & 1<<5);		// Ждать выключения насоса
				_delay_ms(10);
				PORTB &= ~(1<<0);			// Конец импульса на выключение
			}
			if (rec_byte[2] & 1<<2) {								// Запрос уровня воды
				waterLevel = waterProcent(UZD_distance(), 20);		// Замер уровня воды (в процентах)
			}
			if (rec_byte[2] & 1<<3) {								// Запрос давления
				waterPressure = convert_kPa_in_atm(SPI_read());		// Замер давления воды (в атм. bbbb,bbbb)
			}
			if (rec_byte[2] & 1<<4) {								// Запрос статуса
				pumpStatus = (~PIND & 1<<5)?	1 : 0;				// Считать состояние насоса
			}
			
			// Отправка данных в UART
			uart_transmit_byte(0x7E);
			uint8_t byte2 = pumpStatus<<7;
			uart_transmit_byte(byte2);
			uart_transmit_byte(waterPressure);
			uart_transmit_byte(waterLevel);
			uart_transmit_byte(0x7E + byte2 + waterPressure + waterLevel);
			while ( !(UCSRA & (1<<TXC)) );		// Ждем завершения передачи
			UCSRA |= 1<<TXC;					// Сбрасываем флаг завершения передачи
			
			_delay_us(200);
			PORTB &= ~(1<<1);			// MAX485 на прием
			_delay_us(200);
			UCSRB |= (1<<RXCIE);		// Вкл прерывание по приему
		}
		//////////////////////////////////////////////////////////////////////////
		
		
		asm("wdr");		// Сбросить WDT (обнулить значение)
    }
}



/************************************************************************/
/*                         Описания функций                             */
/************************************************************************/

void uart_init() {											// Функция инициализации UART
	UCSRB = (1<<TXEN|1<<RXEN|1<<RXCIE);						// Включили передатчик и приемник, вкл прерывание по приему
	UCSRC = (1<<UCSZ1|1<<UCSZ0);							// Формат посылки - 8 бит, 1 стоп-бит, проверки на четность нет
	UBRRL = (uint8_t)(SPEED & 0xFF);						// Скорость задается макросом BAUD (см. строку 15)
	UBRRH = (uint8_t)(SPEED >> 8);							// Записываем в регистр скорости передачи UART константу SPEED (см. строку 14)
}

uint8_t uart_receive_byte() {								// Функция приема байта по UART
	while ( !(UCSRA & (1<<RXC)) );							// Ожидание прихода байта
	return UDR;												// Возвращение принятого байта
}

void uart_transmit_byte(uint8_t byte) {						// Функция передачи байта по UART
	while ( !(UCSRA & (1<<UDRE)) );							// Ожидание готовности UART к передаче
	UDR = byte;												// Запись в регистр UDR байта данных начинает процесс передачи
}

/* Измерение уровня воды в емкости (лежачий цилиндр с окружностью диаметром 86см)
 * Возвращает уровень в процентах
 * distance - расстояние до поверхности измеренное датчиком
 * sensorHigh - высота датчика над 100%-м уровнем.
 */ 
uint8_t waterProcent(const uint8_t distance, const uint8_t sensorHigh) {
	if (distance<sensorHigh) return 100;
	if (distance<sensorHigh+8) return 95;
	if (distance<sensorHigh+13) return 90;
	if (distance<sensorHigh+17) return 85;
	if (distance<sensorHigh+21) return 80;
	if (distance<sensorHigh+25) return 75;
	if (distance<sensorHigh+29) return 70;
	if (distance<sensorHigh+32) return 65;
	if (distance<sensorHigh+36) return 60;
	if (distance<sensorHigh+39) return 55;
	if (distance<sensorHigh+43) return 50;
	if (distance<sensorHigh+46) return 45;
	if (distance<sensorHigh+49) return 40;
	if (distance<sensorHigh+53) return 35;
	if (distance<sensorHigh+56) return 30;
	if (distance<sensorHigh+60) return 25;
	if (distance<sensorHigh+64) return 20;
	if (distance<sensorHigh+68) return 15;
	if (distance<sensorHigh+72) return 10;
	if (distance<sensorHigh+77) return 5;
	//if (distance<sensorHigh+86) return 0;
	return 0;
}