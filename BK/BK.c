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
uint8_t	uart_receiveByte();
void	uart_transmitByte(uint8_t transmitByte);
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
	spi_init();
	ultrasonicModule_init();
	
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
				waterLevel = waterProcent(ultrasonicModule_work(), 20);		// ����� ������ ���� (� ���������)
			}
			if (rec_byte[2] & 1<<3) {								// ������ ��������
				waterPressure = convert_kPaToAtm(spiReadData());	// ����� �������� ���� (� ���. bbbb,bbbb)
			}
			if (rec_byte[2] & 1<<4) {								// ������ �������
				pumpStatus = (~PIND & 1<<5)?	1 : 0;				// ������� ��������� ������
			}
			
			// �������� ������ � UART
			tran_byte[1] = 0x7E;			uart_transmitByte(tran_byte[1]);
			tran_byte[2] = pumpStatus<<7;	uart_transmitByte(tran_byte[2]);
			tran_byte[3] = waterPressure;	uart_transmitByte(tran_byte[3]);
			tran_byte[4] = waterLevel;		uart_transmitByte(tran_byte[4]);
			// ������� ����������� �����
			uint8_t crc8 = 0xFF;
			for (uint8_t i=1; i<=4; i++) {
				crc8 = _crc8_ccitt_update(crc8, tran_byte[i]);
			}
											uart_transmitByte(tran_byte[5]);
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

uint8_t uart_receiveByte() {								// ������� ������ ����� �� UART
	while ( !(UCSRA & (1<<RXC)) );							// �������� ������� �����
	return UDR;												// ����������� ��������� �����
}

void uart_transmitByte(uint8_t transmitByte) {				// ������� �������� ����� �� UART
	while ( !(UCSRA & (1<<UDRE)) );							// �������� ���������� UART � ��������
	UDR = transmitByte;										// ������ � ������� UDR ����� ������ �������� ������� ��������
}

/* ��������� ������ ���� � ������� (������� ������� � ����������� ��������� 86��)
 * ���������� ������� � ���������
 * distance - ���������� �� ����������� ���������� ��������
 * sensorHigh - ������ ������� ��� 100%-� �������.
 */ 
uint8_t waterProcent(const uint8_t distance, const uint8_t sensorHeight) {
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
	//if (distance < sensorHeight+86) return 0;
	return 0;
}