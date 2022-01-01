/*
 * PU.c
 * Fclk=4MHz
 * Created: 08.11.2020 17:42:49
 * Author : MM
 */ 

#define	F_CPU	4000000UL		// Тактовая частота микроконтроллера

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <util/crc16.h>
#include "UART.h"
#include "HD44780.h"
#include "PU.h"


int main(void)
{	
	ports_init();
	HD44780_init();
    uart_init();
	//HD44780_init_proteus()
	
	printString[0]("Запрос данных...");		// Вывод начального текста на дисплей
	
	asm("sei");								// Разрешить глобальные прерывания
	startInformationExchange();				// Начать информационный обмен. Отправка/прием данных
	//asm("cli");							// Запретить глобальные прерывания
	
	
    while (1) 
    {	
		if (flag.recMessageOK) {			// Если сообщение принято успешно - отобразить данные
			flag.recMessageOK = 0;			// Сбросить флаг успешного приема
			rec_message();					// Расшифровка принятого сообщения
			
			// Обработка данных для вывода на дисплей			
			uint8_t level[] = {	(data.watterLevel/100) ? (data.watterLevel/100)+48 : (uint8_t)' ',
								(data.watterLevel/10) ? ((data.watterLevel/10)%10)+48 : (uint8_t)' ',
								(data.watterLevel%10)+48 };
			uint8_t pressure[] = {	(data.watterPressure>>4)+48, (uint8_t)',',
									(data.watterPressure & 0x0F)+48 };
									
			// Вывод данных на дисплей
			if (!flag.recMessagePRE) {		// Если предыдущее сообщение принято с ошибками
				printString[1]("Уровень"); print(0xFF); printString[0]("Давление");	// Вывод заголовка
				printString[2]("    %  "); print(0xFF); printString[0]("    атм."); // Вывод единиц измерения	
			}
			printArray_adr(level, 3, 0x41);
			printArray_adr(pressure, 3, 0x49);
						
			// Установка команд для отправки
			if (PINA & 1<<0) {				// Автоматический режим
				if (data.watterLevel<30 && data.pumpStatus!=1)	com.pumpON = 1;		else com.pumpON = 0;
				if (data.watterLevel>90 && data.pumpStatus==1)	com.pumpOFF = 1;	else com.pumpOFF = 0;
			} else {						// Ручной режим
				if (flag.manON && data.pumpStatus!=1)	com.pumpON = 1;		else com.pumpON = 0;
				if (flag.manOFF && data.pumpStatus==1)	com.pumpOFF = 1;	else com.pumpOFF = 0;
			}
			tran_message();					// Сформировать сообщение на отправку
			startInformationExchange();
			flag.recMessagePRE = 1;			// Информация для следующего сообщения
		}
		
		if (flag.recMessageNOK) {			// Если прием выполнен с ошибками
			flag.recMessageNOK = 0;			// Сбросить флаг приема с ошибками
			
			// Вывести на дисплей предупреждение
			if (flag.recMessagePRE)
			printString[1]("    ВНИМАНИЕ    ");
			printString[2](" Неверный прием ");
			
			// Установка команд для отправки
			if (PINA & 1<<0) {				// Автоматический режим
				if (data.watterLevel<30 && data.pumpStatus!=1)	com.pumpON = 1;		else com.pumpON = 0;
				if (data.watterLevel>90 && data.pumpStatus==1)	com.pumpOFF = 1;	else com.pumpOFF = 0;
			} else {						// Ручной режим
				if (flag.manON && data.pumpStatus!=1)	com.pumpON = 1;		else com.pumpON = 0;
				if (flag.manOFF && data.pumpStatus==1)	com.pumpOFF = 1;	else com.pumpOFF = 0;
			}
			tran_message();					// Сформировать сообщение на отправку
			startInformationExchange();
			flag.recMessagePRE = 0;			// Информация для следующего сообщения
		}
		
		// !!! 1 Отладить программу так как есть (юарт, внешние прерывания и прочее)
		// !!! 2 Реализовать ошибки ниже
		
		// Если сообщения принимаются с ошибками на протяжении 10 мин., то ошибка
		// ...Если связи нет более 10 мин., то ошибка
		// ...Если команда на ВКЛ, а статус ВЫКЛ более 5 мин., то выкл и и ошибка
		// ...Если насос включен и давления нет, то выкл и ошибка
		// ...Если команда на ВЫКЛ, а статус ВКЛ более 5 мин., то выкл и и ошибка
		
    }
}




/************************************************************************/
/*                   Описания функций программы                         */
/************************************************************************/

void ports_init() {
	DDRA	= 0b11111100;
	PORTA	= 0b00000011;
	DDRB	= 0b00000000;
	PORTB	= 0b11111111;
	DDRC	= 0b00000000;
	PORTC	= 0b11111111;
	DDRD	= 0b11110000;
	PORTD	= 0b00011111;
	//MCUSR	= 0b00001010;			// Режим работы внешних прерываний INT0, INT1 по заднему фронту	
	GICR	|= 1<<INT0 | 1<<INT1;	// Включить внешние прерывания INT0, INT1
}

void tran_message() {	// Формирование передаваемого сообщения
	tran_byte[1] = 0x7E;								// Первый байт - Заголовок
	tran_byte[2] =	com.pumpStatus<<4 | com.watterPressure<<3 | com.watterLevel<<2 |
					com.pumpOFF<<1 | com.pumpON<<0;		// Второй байт - Набор команд
	// Считаем контрольную сумму
	uint8_t crc8 = 0xFF;
	for (uint8_t i=1; i<=2; i++) {
		crc8 = _crc8_ccitt_update(crc8, tran_byte[i]);
	}
	tran_byte[3] = crc8;								// Третий байт - контрольная сумма
}

void rec_message() {	// Расшифровка принятого сообщения
	data.pumpStatus = (rec_byte[2] & 1<<7)? 1 : 0;		// Состояние насоса (вкл/выкл)
	data.watterPressure = rec_byte[3];					// Давление насоса в атм. bbbb,bbbb
	data.watterLevel = rec_byte[4];						// Уровень воды в см (расстояние от датчика до поверхности)
}