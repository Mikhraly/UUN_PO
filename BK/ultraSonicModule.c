/*
 * ultraSonicModule.c
 * Created: 09.03.2020 18:05:56
 */ 

#define	F_CPU	4000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "ultraSonicModule.h"

void ultrasonicModule_init() {
	SET(DDR_USM,TRIGGER);					// ����� ��������, ����������� ������ �����������
	RESET(DDR_USM,ECHO);					// ���� ���-�������
}

uint8_t ultrasonicModule_work() {			// ��������� �������� �� 2 ������!
	SET(PORT_USM,TRIGGER);					// ������ ������������ ��������
	_delay_us(20);
	RESET(PORT_USM,TRIGGER);				// ����� ������������ ��������
	while ( !(CHECK(PIN_USM,ECHO)) );		// ���� ���-������� ��� - �����
	TCNT1 = 0;								// ������ ��������� ������������ ���-�������
	while ( CHECK(PIN_USM,ECHO) );			// ���� ���-������� �� ���������� - �����
	return ( (uint8_t)((TCNT1*2)/58 + 1) );	// ���������� ���������� � �����������. �������� ��������� - 1 �� ��-�� ������� �� 58.
											// +1 ��������� �� ������ ������
}