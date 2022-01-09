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
	// Настройка выводов на выход
	DDR_HD44780 |= 1<<E | 1<<RS | 1<<HD44780_D4 | 1<<HD44780_D5 | 1<<HD44780_D6 | 1<<HD44780_D7;
	_delay_ms(20);
	// Инициализация согласно документации на МТ-16S2D (ф. МЭЛТ)
	HD44780_com(0b00110000);	// Установка разрядность интерфейса
	HD44780_com(0b00110000);	// Установка разрядность интерфейса
	HD44780_com(0b00110000);	// Установка разрядность интерфейса
	HD44780_com(0b00100000);	// Установка разрядность интерфейса
	HD44780_com(0b00100000);	// Установка параметров
	HD44780_com(0b10000000);	// Установка параметров
	HD44780_com(0b00000000);	// Выключение дисплея
	HD44780_com(0b10000000);	// Выключение дисплея
	HD44780_com(0b00000000);	// Очистка дисплея
	HD44780_com(0b00010000);	// Очистка дисплея
	HD44780_com(0b00000000);	// Установка режима ввода данных
	HD44780_com(0b01100000);	// Установка режима ввода данных
	// Своя инициализация
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
	// Настройка выводов на выход
	DDR_HD44780 |= 1<<E | 1<<RS | 1<<HD44780_D4 | 1<<HD44780_D5 | 1<<HD44780_D6 | 1<<HD44780_D7;
	HD44780_com(0b00000010);	// Установка дисплея в начальное положение. Возврат курсора
	HD44780_com(0b00101000);	// Параметры интерфейса: Разрядность 4 бита. 2 строки. Размер шрифта 5х7
	HD44780_com(0b00001100);	// Включение дисплея. Запрет видимости курсора. Запрет мерцания курсора
}

void HD44780(const uint8_t mode, const uint8_t byte) {
	// Установить режим команда(0)/данные(1)
	mode ? (PORT_HD44780 |= (1<<RS))	:	(PORT_HD44780 &= ~(1<<RS));
	// Вывод в порт старшей тетрады
	byte&(1<<7) ? (PORT_HD44780 |= (1<<HD44780_D7)) : (PORT_HD44780 &= ~(1<<HD44780_D7));
	byte&(1<<6) ? (PORT_HD44780 |= (1<<HD44780_D6)) : (PORT_HD44780 &= ~(1<<HD44780_D6));
	byte&(1<<5) ? (PORT_HD44780 |= (1<<HD44780_D5)) : (PORT_HD44780 &= ~(1<<HD44780_D5));
	byte&(1<<4) ? (PORT_HD44780 |= (1<<HD44780_D4)) : (PORT_HD44780 &= ~(1<<HD44780_D4));
	// Выдача тактового сиглала
	PORT_HD44780 |= (1<<E);
	asm("nop");
	PORT_HD44780 &= ~(1<<E);
	// Вывод в порт младшей тетрады
	byte&(1<<3) ? (PORT_HD44780 |= (1<<HD44780_D7)) : (PORT_HD44780 &= ~(1<<HD44780_D7));
	byte&(1<<2) ? (PORT_HD44780 |= (1<<HD44780_D6)) : (PORT_HD44780 &= ~(1<<HD44780_D6));
	byte&(1<<1) ? (PORT_HD44780 |= (1<<HD44780_D5)) : (PORT_HD44780 &= ~(1<<HD44780_D5));
	byte&(1<<0) ? (PORT_HD44780 |= (1<<HD44780_D4)) : (PORT_HD44780 &= ~(1<<HD44780_D4));
	// Выдача тактового сиглала
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
		case 'А':	string[i]=0x41;		break;
		case 'Б':	string[i]=0xA0;		break;
		case 'В':	string[i]=0x42;		break;
		case 'Г':	string[i]=0xA1;		break;
		case 'Д':	string[i]=0xE0;		break;
		case 'Е':	string[i]=0x45;		break;
		case 'Ё':	string[i]=0xA2;		break;
		case 'Ж':	string[i]=0xA3;		break;
		case 'З':	string[i]=0xA4;		break;
		case 'И':	string[i]=0xA5;		break;
		case 'Й':	string[i]=0xA6;		break;
		case 'К':	string[i]=0x4B;		break;
		case 'Л':	string[i]=0xA7;		break;
		case 'М':	string[i]=0x4D;		break;
		case 'Н':	string[i]=0x48;		break;
		case 'О':	string[i]=0x4F;		break;
		case 'П':	string[i]=0xA8;		break;
		case 'Р':	string[i]=0x50;		break;
		case 'С':	string[i]=0x43;		break;
		case 'Т':	string[i]=0x54;		break;
		case 'У':	string[i]=0xA9;		break;
		case 'Ф':	string[i]=0xAA;		break;
		case 'Х':	string[i]=0x58;		break;
		case 'Ц':	string[i]=0xE1;		break;
		case 'Ч':	string[i]=0xAB;		break;
		case 'Ш':	string[i]=0xAC;		break;
		case 'Щ':	string[i]=0xE2;		break;
		case 'Ъ':	string[i]=0xAD;		break;
		case 'Ы':	string[i]=0xAE;		break;
		case 'Ь':	string[i]=0x62;		break;
		case 'Э':	string[i]=0xAF;		break;
		case 'Ю':	string[i]=0xB0;		break;
		case 'Я':	string[i]=0xB1;		break;
		case 'а':	string[i]=0x61;		break;
		case 'б':	string[i]=0xB2;		break;
		case 'в':	string[i]=0xB3;		break;
		case 'г':	string[i]=0xB4;		break;
		case 'д':	string[i]=0xE3;		break;
		case 'е':	string[i]=0x65;		break;
		case 'ё':	string[i]=0xB5;		break;
		case 'ж':	string[i]=0xB6;		break;
		case 'з':	string[i]=0xB7;		break;
		case 'и':	string[i]=0xB8;		break;
		case 'й':	string[i]=0xB9;		break;
		case 'к':	string[i]=0xBA;		break;
		case 'л':	string[i]=0xBB;		break;
		case 'м':	string[i]=0xBC;		break;
		case 'н':	string[i]=0xBD;		break;
		case 'о':	string[i]=0x6F;		break;
		case 'п':	string[i]=0xBE;		break;
		case 'р':	string[i]=0x70;		break;
		case 'с':	string[i]=0x63;		break;
		case 'т':	string[i]=0xBF;		break;
		case 'у':	string[i]=0x79;		break;
		case 'ф':	string[i]=0xE4;		break;
		case 'х':	string[i]=0x78;		break;
		case 'ц':	string[i]=0xE5;		break;
		case 'ч':	string[i]=0xC0;		break;
		case 'ш':	string[i]=0xC1;		break;
		case 'щ':	string[i]=0xE6;		break;
		case 'ъ':	string[i]=0xC2;		break;
		case 'ы':	string[i]=0xC3;		break;
		case 'ь':	string[i]=0xC4;		break;
		case 'э':	string[i]=0xC5;		break;
		case 'ю':	string[i]=0xC6;		break;
		case 'я':	string[i]=0xC7;		break;
		default:	break;
		}
	}
}

char HD44780_convertSymbolRus(char symbol) {
	switch(symbol) {
	case 'А':	return 0x41;
	case 'Б':	return 0xA0;
	case 'В':	return 0x42;
	case 'Г':	return 0xA1;
	case 'Д':	return 0xE0;
	case 'Е':	return 0x45;
	case 'Ё':	return 0xA2;
	case 'Ж':	return 0xA3;
	case 'З':	return 0xA4;
	case 'И':	return 0xA5;
	case 'Й':	return 0xA6;
	case 'К':	return 0x4B;
	case 'Л':	return 0xA7;
	case 'М':	return 0x4D;
	case 'Н':	return 0x48;
	case 'О':	return 0x4F;
	case 'П':	return 0xA8;
	case 'Р':	return 0x50;
	case 'С':	return 0x43;
	case 'Т':	return 0x54;
	case 'У':	return 0xA9;
	case 'Ф':	return 0xAA;
	case 'Х':	return 0x58;
	case 'Ц':	return 0xE1;
	case 'Ч':	return 0xAB;
	case 'Ш':	return 0xAC;
	case 'Щ':	return 0xE2;
	case 'Ъ':	return 0xAD;
	case 'Ы':	return 0xAE;
	case 'Ь':	return 0x62;
	case 'Э':	return 0xAF;
	case 'Ю':	return 0xB0;
	case 'Я':	return 0xB1;
	case 'а':	return 0x61;
	case 'б':	return 0xB2;
	case 'в':	return 0xB3;
	case 'г':	return 0xB4;
	case 'д':	return 0xE3;
	case 'е':	return 0x65;
	case 'ё':	return 0xB5;
	case 'ж':	return 0xB6;
	case 'з':	return 0xB7;
	case 'и':	return 0xB8;
	case 'й':	return 0xB9;
	case 'к':	return 0xBA;
	case 'л':	return 0xBB;
	case 'м':	return 0xBC;
	case 'н':	return 0xBD;
	case 'о':	return 0x6F;
	case 'п':	return 0xBE;
	case 'р':	return 0x70;
	case 'с':	return 0x63;
	case 'т':	return 0xBF;
	case 'у':	return 0x79;
	case 'ф':	return 0xE4;
	case 'х':	return 0x78;
	case 'ц':	return 0xE5;
	case 'ч':	return 0xC0;
	case 'ш':	return 0xC1;
	case 'щ':	return 0xE6;
	case 'ъ':	return 0xC2;
	case 'ы':	return 0xC3;
	case 'ь':	return 0xC4;
	case 'э':	return 0xC5;
	case 'ю':	return 0xC6;
	case 'я':	return 0xC7;
	default:	return 0;
	}
}