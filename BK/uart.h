/*
 * uart.h
 *
 * Created: 02.01.2022 23:18:29
 *  Author: gasra
 */ 


#ifndef UART_H_
#define UART_H_


#ifndef F_CPU
#define	F_CPU	4000000UL									// Тактовая частота микроконтроллера
#endif

#include <avr/io.h>

#define	BAUD	9600UL										// Скорость обмена по UART
#define SPEED	(F_CPU/(BAUD*16)-1)							// Константа для записи в регистр скорости передачи UART

void	uart_init();
uint8_t uart_receiveByte();
void	uart_transmitByte(uint8_t transmitByte);


#endif /* UART_H_ */