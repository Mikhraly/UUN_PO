/*
 * ultraSonicModule.c
 * Created: 09.03.2020 18:05:56
 */ 


#include "ultraSonicModule.h"

void ultrasonicModule_init()	{
	SET(DDR_USM,TRIGGER);			// ����� ��������, ����������� ������ �����������
	RESET(DDR_USM,ECHO);			// ���� ���-�������
}

uint8_t ultrasonicModule_work() {	// ��������� �������� �� 2 ������!
	SET(PORT_USM,TRIGGER);			// ������ ������������ ��������
	delay_us(20);
	RESET(PORT_USM,TRIGGER);		// ����� ������������ ��������
	while (0==CHECK(PIN_USM,ECHO));	// ���� ���-������� ��� - �����
	TCNT1 = 0;						// ������ ��������� ������������ ���-�������
	while (1==CHECK(PIN_USM,ECHO));	// ���� ���-������� �� ���������� - �����
	return ( (TCNT1*2)/58 + 1 );	// ���������� ���������� � �����������. �������� ��������� - 1 �� ��-�� ������� �� 58.
									// +1 ��������� �� ������ ������
}