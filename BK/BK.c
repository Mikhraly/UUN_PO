/*
 * BK.c
 * Fclk=4MHz
 * Created: 14.12.2019 19:32:54
 */
#define	F_CPU	4000000UL									// �������� ������� ����������������

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <util/crc16.h>
#include "spi_for_MCP3201.h"
#include "UZD.h"

#define	BAUD	9600UL										// �������� ������ �� UART
#define SPEED	(F_CPU/(BAUD*16)-1)							// ��������� ��� ������ � ������� �������� �������� UART

volatile uint8_t	num = 1;								// ����� ��������� ����� �� UART
volatile uint8_t	rec_byte[4];							// ������ ����������� ����. [0] - �� ������������
volatile uint8_t	tran_byte[6];							// ������ ������������ ����. [0] - �� ������������
volatile uint8_t	recMessageOK;							// ���� ���������� ������ ���������


ISR (USART_RX_vect) {										// ������� ������ ����� �� UART ����� ����������
	rec_byte[num] = UDR;									// ������� ������
	if(num==1 && rec_byte[1] != 0x7E) return;
	if(num == 3) {
		num = 1;
		uint8_t crc8 = 0xFF;
		for (uint8_t i=1; i<=2; i++) {
			crc8 = _crc8_ccitt_update(crc8, rec_byte[i]);
		}
		if(crc8 == rec_byte[3]) {
			recMessageOK = 1;
			UCSRB &= ~(1<<RXCIE);	// ���� ���������� �� ������
		}
		else return;				// ���� �� �� �������, ����� � ������ �������
	}
	else num++;
}

void	uart_init();
uint8_t	uart_receive_byte();
void	uart_transmit_byte(uint8_t);
uint8_t waterProcent(const uint8_t distance, const uint8_t sensorHigh);


int main(void)
{
	asm("cli");							// ���� ���������� ����������
	asm("wdr");							// �������� WDT (�������� ��������)
	MCUSR &= ~(1<<WDRF);				// �������� ���� ������ WDT
	WDTCR |= (1<<WDCE) | (1<<WDE);		// ��� WDT � ������ ���������� ������
	WDTCR = (1<<WDE) | (1<<WDP3);		// ����� �� ������ - 4 �������
	
	
	uint8_t		waterLevel = 0;			// ���������� ��� �������� ������ ���� � �����������
	uint8_t		waterPressure = 0;		// ���������� ��� �������� �������� ���� � ��� (�������� 10 ���)
	uint8_t		pumpStatus = 0;			// ���������� ��� �������� ��������� ������ (���/����)
	
	uart_init();
	SPI_init();
	UZD_init();
	
	DDRD &= ~(1<<5);		// ���� ��� ���������� ��������� ������ ���/����
	DDRD |=  (1<<6);		// ����� ������� ���
	DDRB |=  (1<<0);		// ����� ������� ����
	DDRB |=  (1<<1);		// ����� ������� �����/�������� �� MAX485
	PORTD |=  (1<<5);		// ���������� ������������� ��������
	PORTD &= ~(1<<6);		// ��� ������� ���
	PORTB &= ~(1<<0);		// ��� ������� ����
	PORTB &= ~(1<<1);		// MAX485 �� �����
	
	asm("sei");	// ��� ���������� ���������� ��� ������ ��������� UART
	
	// ���������� ������
	PORTB |= 1<<0;			// ������ �������� �� ����������
	while (~PIND & 1<<5);	// ����� ���������� ������
	PORTB &= ~(1<<0);		// ����� �������� �� ����������
	
    while (1) 
    {
		/************************************************************************/
		/*                    ��������� ��������� �� UART                       */
		/************************************************************************/
		if (recMessageOK) {					// ���� ����� ��������� �������� �������
			recMessageOK = 0;				// �������� ���� ��������� ������
			UCSRB &= ~(1<<RXCIE);			// ���� ���������� UART �� ������
			PORTB |= 1<<1;					// MAX485 �� ��������
			_delay_us(200);
			
			if (rec_byte[2] & 1<<0) {		// ������ �� ���
				PORTD |= 1<<6;				// ������ �������� �� ���������
				while (PIND & 1<<5);		// ����� ��������� ������
				_delay_ms(500);
				PORTD &= ~(1<<6);			// ����� �������� �� ���������
			}
			if (rec_byte[2] & 1<<1) {		// ������ �� ����
				PORTB |= 1<<0;				// ������ �������� �� ����������
				while (~PIND & 1<<5);		// ����� ���������� ������
				_delay_ms(10);
				PORTB &= ~(1<<0);			// ����� �������� �� ����������
			}
			if (rec_byte[2] & 1<<2) {								// ������ ������ ����
				waterLevel = waterProcent(UZD_distance(), 20);		// ����� ������ ���� (� ���������)
			}
			if (rec_byte[2] & 1<<3) {								// ������ ��������
				waterPressure = convert_kPa_in_atm(SPI_read());		// ����� �������� ���� (� ���. bbbb,bbbb)
			}
			if (rec_byte[2] & 1<<4) {								// ������ �������
				pumpStatus = (~PIND & 1<<5)?	1 : 0;				// ������� ��������� ������
			}
			
			// �������� ������ � UART
			tran_byte[1] = 0x7E;			uart_transmit_byte(tran_byte[1]);
			tran_byte[2] = pumpStatus<<7;	uart_transmit_byte(tran_byte[2]);
			tran_byte[3] = waterPressure;	uart_transmit_byte(tran_byte[3]);
			tran_byte[4] = waterLevel;		uart_transmit_byte(tran_byte[4]);
			// ������� ����������� �����
			uint8_t crc8 = 0xFF;
			for (uint8_t i=1; i<=4; i++) {
				crc8 = _crc8_ccitt_update(crc8, tran_byte[i]);
			}
											uart_transmit_byte(tran_byte[5]);
			while ( !(UCSRA & (1<<TXC)) );		// ���� ���������� ��������
			UCSRA |= 1<<TXC;					// ���������� ���� ���������� ��������
			
			_delay_us(200);
			PORTB &= ~(1<<1);			// MAX485 �� �����
			_delay_us(200);
			UCSRB |= (1<<RXCIE);		// ��� ���������� �� ������
		}
		//////////////////////////////////////////////////////////////////////////
		
		
		asm("wdr");		// �������� WDT (�������� ��������)
    }
}



/************************************************************************/
/*                         �������� �������                             */
/************************************************************************/

void uart_init() {											// ������� ������������� UART
	UCSRB = (1<<TXEN|1<<RXEN|1<<RXCIE);						// �������� ���������� � ��������, ��� ���������� �� ������
	UCSRC = (1<<UCSZ1|1<<UCSZ0);							// ������ ������� - 8 ���, 1 ����-���, �������� �� �������� ���
	UBRRL = (uint8_t)(SPEED & 0xFF);						// �������� �������� �������� BAUD (��. ������ 15)
	UBRRH = (uint8_t)(SPEED >> 8);							// ���������� � ������� �������� �������� UART ��������� SPEED (��. ������ 14)
}

uint8_t uart_receive_byte() {								// ������� ������ ����� �� UART
	while ( !(UCSRA & (1<<RXC)) );							// �������� ������� �����
	return UDR;												// ����������� ��������� �����
}

void uart_transmit_byte(uint8_t byte) {						// ������� �������� ����� �� UART
	while ( !(UCSRA & (1<<UDRE)) );							// �������� ���������� UART � ��������
	UDR = byte;												// ������ � ������� UDR ����� ������ �������� ������� ��������
}

/* ��������� ������ ���� � ������� (������� ������� � ����������� ��������� 86��)
 * ���������� ������� � ���������
 * distance - ���������� �� ����������� ���������� ��������
 * sensorHigh - ������ ������� ��� 100%-� �������.
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