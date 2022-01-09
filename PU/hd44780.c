/*
 * HD44780.c
 *
 * Created: 06.02.2021 21:54:35
 *  Author: gasra
 */ 

#include "hd44780.h"

void HD44780(const uint8_t mode, const uint8_t byte);
void HD44780_convertStringRus(char string[]);
char HD44780_convertSymbolRus(char symbol);

void HD44780_init() {
	// ��������� ������� �� �����
	DDR_HD44780 |= 1<<E | 1<<RS | 1<<HD44780_D4 | 1<<HD44780_D5 | 1<<HD44780_D6 | 1<<HD44780_D7;
	_delay_ms(20);
	// ������������� �������� ������������ �� ��-16S2D (�. ����)
	HD44780_com(0b00110000);	// ��������� ����������� ����������
	HD44780_com(0b00110000);	// ��������� ����������� ����������
	HD44780_com(0b00110000);	// ��������� ����������� ����������
	HD44780_com(0b00100000);	// ��������� ����������� ����������
	HD44780_com(0b00100000);	// ��������� ����������
	HD44780_com(0b10000000);	// ��������� ����������
	HD44780_com(0b00000000);	// ���������� �������
	HD44780_com(0b10000000);	// ���������� �������
	HD44780_com(0b00000000);	// ������� �������
	HD44780_com(0b00010000);	// ������� �������
	HD44780_com(0b00000000);	// ��������� ������ ����� ������
	HD44780_com(0b01100000);	// ��������� ������ ����� ������
	// ���� �������������
	HD44780_com(0b00000011);
	HD44780_com(0b00000011);
	HD44780_com(0b00000011);
	HD44780_com(0b00101000);
	HD44780_com(0b00101000);
	HD44780_com(0b00001000);
	HD44780_com(0b00000001); _delay_ms(10);
	HD44780_com(0b00010000);
	HD44780_com(0b00000110);
	HD44780_com(0b00000010);
	HD44780_com(0b00101000);
	HD44780_com(0b00001100);
}

void HD44780_init_proteus() {
	// ��������� ������� �� �����
	DDR_HD44780 |= 1<<E | 1<<RS | 1<<HD44780_D4 | 1<<HD44780_D5 | 1<<HD44780_D6 | 1<<HD44780_D7;
	HD44780_com(0b00000010);	// ��������� ������� � ��������� ���������. ������� �������
	HD44780_com(0b00101000);	// ��������� ����������: ����������� 4 ����. 2 ������. ������ ������ 5�7
	HD44780_com(0b00001100);	// ��������� �������. ������ ��������� �������. ������ �������� �������
}

void HD44780(const uint8_t mode, const uint8_t byte) {
	// ���������� ����� �������(0)/������(1)
	mode ? (PORT_HD44780 |= (1<<RS))	:	(PORT_HD44780 &= ~(1<<RS));
	// ����� � ���� ������� �������
	byte&(1<<7) ? (PORT_HD44780 |= (1<<HD44780_D7)) : (PORT_HD44780 &= ~(1<<HD44780_D7));
	byte&(1<<6) ? (PORT_HD44780 |= (1<<HD44780_D6)) : (PORT_HD44780 &= ~(1<<HD44780_D6));
	byte&(1<<5) ? (PORT_HD44780 |= (1<<HD44780_D5)) : (PORT_HD44780 &= ~(1<<HD44780_D5));
	byte&(1<<4) ? (PORT_HD44780 |= (1<<HD44780_D4)) : (PORT_HD44780 &= ~(1<<HD44780_D4));
	// ������ ��������� �������
	PORT_HD44780 |= (1<<E);
	asm("nop");
	PORT_HD44780 &= ~(1<<E);
	// ����� � ���� ������� �������
	byte&(1<<3) ? (PORT_HD44780 |= (1<<HD44780_D7)) : (PORT_HD44780 &= ~(1<<HD44780_D7));
	byte&(1<<2) ? (PORT_HD44780 |= (1<<HD44780_D6)) : (PORT_HD44780 &= ~(1<<HD44780_D6));
	byte&(1<<1) ? (PORT_HD44780 |= (1<<HD44780_D5)) : (PORT_HD44780 &= ~(1<<HD44780_D5));
	byte&(1<<0) ? (PORT_HD44780 |= (1<<HD44780_D4)) : (PORT_HD44780 &= ~(1<<HD44780_D4));
	// ������ ��������� �������
	PORT_HD44780 |= (1<<E);
	asm("nop");
	PORT_HD44780 &= ~(1<<E);
	_delay_ms(10);
}

void HD44780_com(const uint8_t command) {
	HD44780(0, command);
	_delay_us(40);
}

void HD44780_adr(uint8_t adress) {
	adress |= 1<<7;
	HD44780(0, adress);
	_delay_us(40);
}


void HD44780_print(const uint8_t data) {
	HD44780(1, data);
	_delay_us(40);
}

void HD44780_print_adr(const uint8_t data, uint8_t adress) {
	HD44780_adr(adress);
	HD44780(1, data);
	_delay_us(40);
}


void HD44780_printArray(const uint8_t *array, const uint8_t size) {
	for (uint8_t i=0; i<size; i++)	HD44780_print(*(array+i));
}

void HD44780_printArray1(const uint8_t *array, const uint8_t size) {
	HD44780_adr(0x00);
	for (uint8_t i=0; i<size; i++)	HD44780_print(*(array+i));
}

void HD44780_printArray2(const uint8_t *array, const uint8_t size) {
	HD44780_adr(0x40);
	for (uint8_t i=0; i<size; i++)	HD44780_print(*(array+i));
}

void HD44780_printArray_adr(const uint8_t *array, const uint8_t size, uint8_t adress) {
	HD44780_adr(adress);
	for (uint8_t i=0; i<size; i++)	HD44780_print(*(array+i));
}


void HD44780_printString(char *string) {
	HD44780_convertStringRus(string);
	for (uint8_t i=0; string[i] != '\0'; i++)	HD44780_print(string[i]);
}

void HD44780_printString1(char *string) {
	HD44780_adr(0x00);
	HD44780_printString(string);
}

void HD44780_printString2(char *string) {
	HD44780_adr(0x40);
	HD44780_printString(string);
}

void HD44780_printString_adr(char *string, uint8_t adress) {
	HD44780_adr(adress);
	HD44780_printString(string);
}

void HD44780_convertStringRus(char string[]) {
	for(uint8_t i=0; string[i] != '\0'; i++) {
		switch(string[i]) {
		case '�':	string[i]=0x41;		break;
		case '�':	string[i]=0xA0;		break;
		case '�':	string[i]=0x42;		break;
		case '�':	string[i]=0xA1;		break;
		case '�':	string[i]=0xE0;		break;
		case '�':	string[i]=0x45;		break;
		case '�':	string[i]=0xA2;		break;
		case '�':	string[i]=0xA3;		break;
		case '�':	string[i]=0xA4;		break;
		case '�':	string[i]=0xA5;		break;
		case '�':	string[i]=0xA6;		break;
		case '�':	string[i]=0x4B;		break;
		case '�':	string[i]=0xA7;		break;
		case '�':	string[i]=0x4D;		break;
		case '�':	string[i]=0x48;		break;
		case '�':	string[i]=0x4F;		break;
		case '�':	string[i]=0xA8;		break;
		case '�':	string[i]=0x50;		break;
		case '�':	string[i]=0x43;		break;
		case '�':	string[i]=0x54;		break;
		case '�':	string[i]=0xA9;		break;
		case '�':	string[i]=0xAA;		break;
		case '�':	string[i]=0x58;		break;
		case '�':	string[i]=0xE1;		break;
		case '�':	string[i]=0xAB;		break;
		case '�':	string[i]=0xAC;		break;
		case '�':	string[i]=0xE2;		break;
		case '�':	string[i]=0xAD;		break;
		case '�':	string[i]=0xAE;		break;
		case '�':	string[i]=0x62;		break;
		case '�':	string[i]=0xAF;		break;
		case '�':	string[i]=0xB0;		break;
		case '�':	string[i]=0xB1;		break;
		case '�':	string[i]=0x61;		break;
		case '�':	string[i]=0xB2;		break;
		case '�':	string[i]=0xB3;		break;
		case '�':	string[i]=0xB4;		break;
		case '�':	string[i]=0xE3;		break;
		case '�':	string[i]=0x65;		break;
		case '�':	string[i]=0xB5;		break;
		case '�':	string[i]=0xB6;		break;
		case '�':	string[i]=0xB7;		break;
		case '�':	string[i]=0xB8;		break;
		case '�':	string[i]=0xB9;		break;
		case '�':	string[i]=0xBA;		break;
		case '�':	string[i]=0xBB;		break;
		case '�':	string[i]=0xBC;		break;
		case '�':	string[i]=0xBD;		break;
		case '�':	string[i]=0x6F;		break;
		case '�':	string[i]=0xBE;		break;
		case '�':	string[i]=0x70;		break;
		case '�':	string[i]=0x63;		break;
		case '�':	string[i]=0xBF;		break;
		case '�':	string[i]=0x79;		break;
		case '�':	string[i]=0xE4;		break;
		case '�':	string[i]=0x78;		break;
		case '�':	string[i]=0xE5;		break;
		case '�':	string[i]=0xC0;		break;
		case '�':	string[i]=0xC1;		break;
		case '�':	string[i]=0xE6;		break;
		case '�':	string[i]=0xC2;		break;
		case '�':	string[i]=0xC3;		break;
		case '�':	string[i]=0xC4;		break;
		case '�':	string[i]=0xC5;		break;
		case '�':	string[i]=0xC6;		break;
		case '�':	string[i]=0xC7;		break;
		default:	break;
		}
	}
}

char HD44780_convertSymbolRus(char symbol) {
	switch(symbol) {
	case '�':	return 0x41;
	case '�':	return 0xA0;
	case '�':	return 0x42;
	case '�':	return 0xA1;
	case '�':	return 0xE0;
	case '�':	return 0x45;
	case '�':	return 0xA2;
	case '�':	return 0xA3;
	case '�':	return 0xA4;
	case '�':	return 0xA5;
	case '�':	return 0xA6;
	case '�':	return 0x4B;
	case '�':	return 0xA7;
	case '�':	return 0x4D;
	case '�':	return 0x48;
	case '�':	return 0x4F;
	case '�':	return 0xA8;
	case '�':	return 0x50;
	case '�':	return 0x43;
	case '�':	return 0x54;
	case '�':	return 0xA9;
	case '�':	return 0xAA;
	case '�':	return 0x58;
	case '�':	return 0xE1;
	case '�':	return 0xAB;
	case '�':	return 0xAC;
	case '�':	return 0xE2;
	case '�':	return 0xAD;
	case '�':	return 0xAE;
	case '�':	return 0x62;
	case '�':	return 0xAF;
	case '�':	return 0xB0;
	case '�':	return 0xB1;
	case '�':	return 0x61;
	case '�':	return 0xB2;
	case '�':	return 0xB3;
	case '�':	return 0xB4;
	case '�':	return 0xE3;
	case '�':	return 0x65;
	case '�':	return 0xB5;
	case '�':	return 0xB6;
	case '�':	return 0xB7;
	case '�':	return 0xB8;
	case '�':	return 0xB9;
	case '�':	return 0xBA;
	case '�':	return 0xBB;
	case '�':	return 0xBC;
	case '�':	return 0xBD;
	case '�':	return 0x6F;
	case '�':	return 0xBE;
	case '�':	return 0x70;
	case '�':	return 0x63;
	case '�':	return 0xBF;
	case '�':	return 0x79;
	case '�':	return 0xE4;
	case '�':	return 0x78;
	case '�':	return 0xE5;
	case '�':	return 0xC0;
	case '�':	return 0xC1;
	case '�':	return 0xE6;
	case '�':	return 0xC2;
	case '�':	return 0xC3;
	case '�':	return 0xC4;
	case '�':	return 0xC5;
	case '�':	return 0xC6;
	case '�':	return 0xC7;
	default:	return 0;
	}
}