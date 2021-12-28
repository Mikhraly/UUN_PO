/*
 * ultraSonicModule.c
 * Created: 09.03.2020 18:05:56
 */ 


#include "ultraSonicModule.h"

void ultrasonicModule_init()	{
	TCCR1A = 0;
	TCCR1B = 0b010;					// Fclk:8, 1 ������: 2 ���
	SET(DDR_USM,TRIGGER);			// ����� ��������, ����������� ������ �����������
	RESET(DDR_USM,ECHO);			// ���� ���-�������
}

uint8_t ultrasonicModule_work {		// ��������� �������� �� 2 ������!
	SET(PORT_USM,TRIGGER);			// ������ ������������ ��������
	TCNT1 = 0;
	while (TCNT1<10);				// �������� 20 ���
	RESET(PORT_USM,TRIGGER);		// ����� ������������ ��������
	while (0==CHECK(PIN_USM,ECHO));	// ���� ���-������� ��� - �����
	TCNT1 = 0;						// ������ ��������� ������������ ���-�������
	while (1==CHECK(PIN_USM,ECHO));	// ���� ���-������� �� ���������� - �����
	return ( (TCNT1*2)/58 + 1 );	// ���������� ���������� � �����������. �������� ��������� - 1 �� ��-�� ������� �� 58. +1 ��������� �� ������ ������
}