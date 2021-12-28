/*
 * ultraSonicModule.h
 * Short name - USM
 * Fclk = 4MHz
 * Created: 09.03.2020 18:06:22
 * Author: Lenovo
 * ��������� ��� ������ ��������������� ������� ����������� HC-SR04.
 * �������� ������� - 2 ��.
 * ��������� ����������� - 3 �� (�������+���������)
 * ����������� ����� ����� ���������� �������� ������� - 30 ���.
 * ������������ ������/������� 1 (16-�� ���������).
 * ������������ ����� ���������� ~23�� (200 ��)
 */ 


#ifndef USM_H_
#define USM_H_


#include <avr/io.h>

#define	DDR_USM		DDRB
#define PORT_USM	PORTB
#define PIN_USM		PINB
#define trigger		4
#define echo		3

#define SET(port,pin)	((port) |= (1<<(pin)))
#define RESET(port,pin)	((port) &= ~(1<<(pin)))
#define CHECK(port,pin)	((port) & (1<<(pin)))

void	ultrasonicModule_init();		// ������� ������������� ������� �����������
uint8_t	ultrasonicModule_work();		// ������� ��������� ���������� � �����������. �� ����� 200 ��!


#endif /* USM_H_*/