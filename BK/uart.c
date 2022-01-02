/*
 * uart.c
 *
 * Created: 02.01.2022 23:18:14
 *  Author: gasra
 */ 

#include "uart.h"

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