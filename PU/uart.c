/*
 * UART.c
 *
 * Created: 07.02.2021 1:13:12
 *  Author: gasra
 */ 

#include "uart.h"

void uart_init() {								// ??????? ????????????? UART
	UCSRB = (1<<TXEN|1<<RXEN);					// ???????? ?????????? ? ????????
	//UCSRB = 1<<UDRIE;							// ???????? ?????????? ?? ???????????? ???????? ?????? (??? ????????)
	//UCSRB = 1<<TXCIE;							// ???????? ?????????? ?? ?????????? ????????
	//UCSRB = 1<<RXCIE;							// ???????? ?????????? ?? ?????????? ??????
	UCSRC = (1<<UCSZ1|1<<UCSZ0);				// ?????? ??????? - 8 ???, 1 ????-???, ???????? ?? ???????? ???
	UBRRL = (uint8_t)(SPEED & 0xFF);			// ???????? ???????? ???????? BAUD (??. ?????? 19 UART.h)
	UBRRH = (uint8_t)(SPEED >> 8);				// ?????????? ? ??????? ???????? ???????? UART ????????? SPEED (??. ?????? 20 UART.h)
}

uint8_t uart_receiveByte() {					// ??????? ?????? ????? ?? UART
	while ( !(UCSRA & (1<<RXC)) );				// ???????? ??????? ?????
	return UDR;									// ??????????? ????????? ?????
}

void uart_transmitByte(uint8_t byte) {			// ??????? ???????? ????? ?? UART
	while ( !(UCSRA & (1<<UDRE)) );				// ???????? ?????????? UART ? ????????
	UDR = byte;									// ?????? ? ??????? UDR ????? ?????? ???????? ??????? ????????
}

