/*
 * HD44780.c
 * ЖК-модуль MT-16S2D ф. МЭЛТ
 * Created: 06.02.2021 21:54:35
 *  Author: gasra
 */ 

#include "hd44780.h"

void hd44780_work(const uint8_t mode, const uint8_t byte);
void hd44780_convertStringRus(char string[]);
char hd44780_convertSymbolRus(char symbol);

void hd44780_init() {
	// Настройка выводов на выход
	DDR_HD44780 |= 1<<E | 1<<RS | 1<<HD44780_D4 | 1<<HD44780_D5 | 1<<HD44780_D6 | 1<<HD44780_D7;
	_delay_ms(20);
	// Инициализация согласно документации на МТ-16S2D (ф. МЭЛТ)
	/*
	hd44780_com(0b00111010);	// Установка разрядность интерфейса
	hd44780_com(0b00111010);	// Установка разрядность интерфейса
	hd44780_com(0b00111010);	// Установка разрядность интерфейса
	hd44780_com(0b00101010);	// Установка разрядность интерфейса
	hd44780_com(0b00101010);	// Установка параметров
	hd44780_com(0b10000000);	// Установка параметров
	hd44780_com(0b00000000);	// Выключение дисплея
	hd44780_com(0b10000000);	// Выключение дисплея
	hd44780_com(0b00000000);	// Очистка дисплея
	hd44780_com(0b00010100);	// Очистка дисплея
	hd44780_com(0b00000000);	// Установка режима ввода данных
	hd44780_com(0b01100000);	// Установка режима ввода данных
	*/
	// Своя инициализация
	_delay_ms(40);
	hd44780_work(0, 0b00000011); _delay_ms(6);
	hd44780_work(0, 0b00000011); _delay_ms(6);
	hd44780_work(0, 0b00000011); _delay_ms(6);
	hd44780_work(0, 0b00101010); _delay_ms(6); // разрядность 4 бита, страница 1
	hd44780_work(0, 0b00101010); _delay_ms(6); // разрядность 4 бита, страница 1
	hd44780_work(0, 0b00001000); _delay_ms(6);
	hd44780_work(0, 0b00000001); _delay_ms(6);
	hd44780_work(0, 0b00010000); _delay_ms(6);
	hd44780_work(0, 0b00000110); _delay_ms(6);
	hd44780_work(0, 0b00000010); _delay_ms(6);
	hd44780_work(0, 0b00101010); _delay_ms(6); // разрядность 4 бита, страница 1
	hd44780_work(0, 0b00001100); _delay_ms(6);
}

void hd44780_init_proteus() {
	// Настройка выводов на выход
	DDR_HD44780 |= 1<<E | 1<<RS | 1<<HD44780_D4 | 1<<HD44780_D5 | 1<<HD44780_D6 | 1<<HD44780_D7;
	hd44780_com(0b00000010);	// Установка дисплея в начальное положение. Возврат курсора
	hd44780_com(0b00101000);	// Параметры интерфейса: Разрядность 4 бита. 2 строки. Размер шрифта 5х7
	hd44780_com(0b00001100);	// Включение дисплея. Запрет видимости курсора. Запрет мерцания курсора
}

void hd44780_work(const uint8_t mode, const uint8_t byte) {
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

void hd44780_com(const uint8_t command) {
	hd44780_work(0, command);
	_delay_us(40);
}

void hd44780_setAddress(uint8_t adress) {
	adress |= 1<<7;
	hd44780_work(0, adress);
	_delay_us(40);
}


void hd44780_print(const uint8_t data) {
	hd44780_work(1, data);
	_delay_us(40);
}


void hd44780_printArray(const uint8_t *array, const uint8_t size) {
	for (uint8_t i=0; i<size; i++)	hd44780_print(*(array+i));
}

void hd44780_printArray1(const uint8_t *array, const uint8_t size) {
	hd44780_setAddress(0x00);
	for (uint8_t i=0; i<size; i++)	hd44780_print(*(array+i));
}

void hd44780_printArray2(const uint8_t *array, const uint8_t size) {
	hd44780_setAddress(0x40);
	for (uint8_t i=0; i<size; i++)	hd44780_print(*(array+i));
}


/* Работает некорректно со страницей 0. Первая печать проходит успешно. При повторном выводе той же строки некоторые символы
 * подставляются со страницы 1. Вероятно баги самого дисплея, но при следующей форме записи повторно строку выводит корректно:
 * setAddress(0x00); print(0x20); print(0x20); print(0x4F); и т.д.
 */
void hd44780_printString(char *string) {
	//hd44780_convertStringRus(string);			// Раскоментировать при использовании "Страницы 0" дисплея МТ-16S2D
	for (uint8_t i=0; string[i] != '\0'; i++)
		hd44780_print(string[i]);
}

void hd44780_printString1(char *string) {
	hd44780_setAddress(0x00);
	hd44780_printString(string);
}

void hd44780_printString2(char *string) {
	hd44780_setAddress(0x40);
	hd44780_printString(string);
}


void hd44780_convertStringRus(char string[]) {
	for(uint8_t i=0; string[i] != '\0'; i++)
		string[i] = hd44780_convertSymbolRus(string[i]);
}

char hd44780_convertSymbolRus(char symbol) {
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
	default:	return symbol;
	}
}