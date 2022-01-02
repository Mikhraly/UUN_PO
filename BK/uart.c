/*
 * uart.c
 *
 * Created: 02.01.2022 23:18:14
 *  Author: gasra
 */ 

#include "uart.h"

void uart_init() {											// Функция инициализации UART
	UCSRB = (1<<TXEN|1<<RXEN|1<<RXCIE);						// Включили передатчик и приемник, вкл прерывание по приему
	UCSRC = (1<<UCSZ1|1<<UCSZ0);							// Формат посылки - 8 бит, 1 стоп-бит, проверки на четность нет
	UBRRL = (uint8_t)(SPEED & 0xFF);						// Скорость задается макросом BAUD (см. строку 15)
	UBRRH = (uint8_t)(SPEED >> 8);							// Записываем в регистр скорости передачи UART константу SPEED (см. строку 14)
}

uint8_t uart_receiveByte() {								// Функция приема байта по UART
	while ( !(UCSRA & (1<<RXC)) );							// Ожидание прихода байта
	return UDR;												// Возвращение принятого байта
}

void uart_transmitByte(uint8_t transmitByte) {				// Функция передачи байта по UART
	while ( !(UCSRA & (1<<UDRE)) );							// Ожидание готовности UART к передаче
	UDR = transmitByte;										// Запись в регистр UDR байта данных начинает процесс передачи
}