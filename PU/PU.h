/*
 * PU.h
 *
 * Created: 15.11.2020 15:00:40
 *  Author: MM
 */ 


#ifndef PU_H_
#define PU_H_


#define	F_CPU	4000000UL		// �������� ������� ����������������

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <util/crc16.h>
#include "uart.h"
#include "hd44780.h"
#include "timers.h"


enum displayAddress {LINE1 = 0, LINE2 = 0x40, LEVEL = 0x41, PRESSURE = 0x49};


struct {					// ��������� ������������ ������
	uint8_t	pumpON			:1;				// ������� �� ��������� ������
	uint8_t	pumpOFF			:1;				// ������� �� ���������� ������
	uint8_t	pumpStatus		:1;				// ������ ��������� ������ (��� ��� ����)
	uint8_t	watterLevel		:1;				// ������ ������ ���� � �������
	uint8_t	watterPressure	:1;				// ������ �������� � ������� ������ ����
} com = {0, 1, 1, 1, 1};

volatile struct {			// ��������� ����������� ������
	uint8_t	pumpStatus		:1;				// ��������� ������ (���/����)
	uint8_t	watterLevel;					// ������ ���� � �������, ��
	uint8_t	watterPressure;					// �������� � ������� ������ ����, ���. bbbb,bbbb
} data = {0};
	
volatile struct {			// ��������� ��������� ������
	uint8_t	tranMessageOK	:1;				// ���� ���������� �������� ���������
	uint8_t	recMessageOK	:1;				// ���� ���������� ������ ���������
	uint8_t	recMessageNOK	:1;				// ���� ���������� ������ ��������� � ��������
	uint8_t recMessagePRE	:1;				// ���� ����������� ���������� ������ (1-OK, 0-NOK)
	uint8_t manON			:1;				// ������ ������� �� ���
	uint8_t manOFF			:1;				// ������ ������� �� ����
	uint8_t myError			:1;
	uint8_t lowWaterLevel	:1;
} flag = {0, 0, 0, 0, 0, 0, 1, 0};

volatile struct {
	uint8_t notOn;
	uint8_t notOff;
	uint8_t notPress;
	uint8_t press;
	uint8_t messageNOK;
	uint8_t connectionNOK;
} myCounters = {0};

volatile uint8_t tran_byte[4];				// ������ ������������ ����. [0] - �� ������������
volatile uint8_t rec_byte[6];				// ������ ����������� ����. [0] - �� ������������


ISR (USART_UDRE_vect) {						// ������� �������� ����� �� UART ����� ����������
	static volatile uint8_t counter = 1;
	UDR = tran_byte[counter];				// ��������� ����
	if (counter == 3) {
		counter = 1;
		UCSRB &= ~(1<<UDRIE);				// ���� ���������� �� ������������ �������� ������
		UCSRB |= 1<<TXCIE;					// ��� ���������� �� ���������� ��������
	}	else counter++;
}

ISR (USART_TXC_vect) {
	flag.tranMessageOK = 1;
	UCSRB &= ~(1<<TXCIE);					// ���� ���������� �� ���������� ��������
	PORTD &= ~(1<<4);						// MAX485 �� �����
	UCSRB |= 1<<RXCIE;						// ��� ���������� �� ���������� ������
}

ISR (USART_RXC_vect) {						// ������� ������ ����� �� UART ����� ����������
	static volatile uint8_t	counter = 1;
	static uint8_t	crc8 = 0xFF;
	
	rec_byte[counter] = UDR;				// ������� ����
	if (counter == 1 && rec_byte[1] != 0x7E) return;
	
	if (counter == 5) {
		counter = 1;
		UCSRB &= ~(1<<RXCIE);				// ���� ���������� �� ������
		if (crc8 == rec_byte[5])	flag.recMessageOK = 1;
		else						flag.recMessageNOK = 1;
		crc8 = 0xFF;
	}
	else crc8 = _crc8_ccitt_update(crc8, rec_byte[counter++]);
}


ISR (INT0_vect) {							// ������ ������� ���
	if (PINA & 1<<0) {						// ���� �������������� �����
		flag.manON = 0;
		flag.manOFF = 0;
	} else {
		flag.manON = 1;
		flag.manOFF = 0;
		_delay_us(200);						// ������ �� �������� ���������
	}
}

ISR (INT1_vect) {							// ������ ������� ����
	if (PINA & 1<<0) {						// ���� �������������� �����
		flag.manON = 0;
		flag.manOFF = 0;
	} else {
		flag.manON = 0;
		flag.manOFF = 1;
		_delay_us(200);						// ������ �� �������� ���������
	}
}


ISR (TIMER1_COMPA_vect) {
	TCNT1 = 0;
	myCounters.notOn = (com.pumpON && !data.pumpStatus) ? myCounters.notOn+1 : 0;
	myCounters.notOff = (com.pumpOFF && data.pumpStatus) ? myCounters.notOff+1 : 0;
	myCounters.notPress = (data.pumpStatus && data.watterPressure < 0x20) ? myCounters.notPress+1 : 0;
	myCounters.press = (!data.pumpStatus && data.watterPressure > 0x19) ? myCounters.press+1 : 0;
	myCounters.messageNOK = (!flag.recMessagePRE) ? myCounters.messageNOK+1 : 0;
	myCounters.connectionNOK++;
}

ISR (TIMER1_COMPB_vect) {
	if (OCR1B == 7812) {
		PORTD |= 1<<6;
		OCR1B = 1;
	} else {
		PORTD &= ~(1<<6);
		OCR1B = 7812;
	}
}


void ports_init();
void encryptionTranMessage();
void decryptionRecMessage();
void startInformationExchange();
void myError(uint8_t errorCode);
void printErrorCode(uint8_t errorCode);

void(*print)(const uint8_t) = hd44780_print;
void(*setAddress)(uint8_t) = hd44780_setAddress;
void(*printArray[])(const uint8_t*, const uint8_t) = { hd44780_printArray, hd44780_printArray1, hd44780_printArray2 };
void(*printString[])(char*) = { hd44780_printString, hd44780_printString1, hd44780_printString2 };


#endif /* PU_H_ */