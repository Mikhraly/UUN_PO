/*
 * HD44780.h
 * ������ ��� ��� ������ � �������� ���� HD44780
 * Created: 06.02.2021 21:54:54
 * Author: gasra
 *
 * ������������ ����������� �� 4-��������� ���� ������
 * ��������� ��������� �������� �� ������ � ������ ������ (����� RW = 0)
 * !!! �����: ����� ������� ������������� ���������� ����������� ������ (��. ����)
 */ 


#ifndef HD44780_H_
#define HD44780_H_

#ifndef F_CPU
#define F_CPU 4000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
		
//////////	!! ����������� ������� !!	//////////
#define	PORT_HD44780	PORTA		// ���� ��� HD44780-����������
#define	DDR_HD44780		DDRA		// ���� DDR ��� HD44780-����������
#define	RS				2			// ����� ����������  RS  (�������(0)/������(1))
#define	E				3			// ����� ����������  E   (����� ������. �������� ��������� � 1 �� 0)
#define	HD44780_D4		4			// ����� ����������  D4
#define	HD44780_D5		5			// ����� ����������  D5
#define	HD44780_D6		6			// ����� ����������  D6
#define	HD44780_D7		7			// ����� ����������  D7
// ������ ���������: �� ������ ���������� RW (������/������) ������ ���� ������ ������� (������)


void hd44780_init();												// �������������
void hd44780_init_proteus();										// ������������� ��� Proteus
void hd44780_com(const uint8_t command);							// ������ ������� � �������
void hd44780_setAddress(uint8_t adress);							// ��������� ������ DDRAM. ����� ������� ������ ��������� �-��� HD44780_print()
void hd44780_print(const uint8_t data);								// ������ ������� � �������
void hd44780_printArray(const uint8_t *array, const uint8_t size);	// ����� �� ������� �������
void hd44780_printArray1(const uint8_t *array, const uint8_t size);
void hd44780_printArray2(const uint8_t *array, const uint8_t size);
void hd44780_printString(char *string);								// ����� �� ������� ������
void hd44780_printString1(char *string);
void hd44780_printString2(char *string);


#endif /* HD44780_H_ */