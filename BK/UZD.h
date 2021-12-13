/*
 * UZD.h
 * Fclk = 4MHz
 * Created: 09.03.2020 18:06:22
 * Author: Lenovo
 * ��������� ��� ������ ��������������� ������� ����������� HC-SR04.
 * �������� ������� - 2 ��.
 * ��������� ����������� - 3 �� (�������+���������)
 * ����������� ����� ����� ���������� �������� ������� - 30 ���.
 * ������������ ������/������� 1 (16-�� ���������).
 * ������������ ����� ���������� ~23��
 */ 


#ifndef UZD_H_
#define UZD_H_


#include <avr/io.h>

#define	DDR_UZD		DDRB
#define PORT_UZD	PORTB
#define PIN_UZD		PINB
#define trigger		4
#define echo		3

#define SET(port,pin)	((port) |= (1<<(pin)))
#define RESET(port,pin)	((port) &= ~(1<<(pin)))
#define CHECK(port,pin)	((port) & (1<<(pin)))

void		UZD_init();			// ������� ������������� ������� �����������
uint8_t		UZD_distance();		// ������� ��������� ���������� � �����������. �� ����� 200 ��!


#endif /* UZD_H_ */