/*
 * PU.h
 *
 * Created: 15.11.2020 15:00:40
 *  Author: MM
 */ 


#ifndef PU_H_
#define PU_H_


typedef struct {	// Структура отправляемых команд
	uint8_t	restart			:1;				// Команда на перезапуск блока коммутации
	uint8_t	pumpStatus		:1;				// Запрос состояния насоса (ВКЛ или ВЫКЛ)
	uint8_t	pumpPressure	:1;				// Запрос давления в системе подачи воды
	uint8_t	watterLevel		:1;				// Запрос уровня воды в емкости
	uint8_t	pumpON			:1;				// Команда на включение насоса
	uint8_t	pumpOFF			:1;				// Команда на выключение насоса
} sCommand;	// Задание краткого имени

typedef struct {	// Структура принимаемых данных
	uint8_t	pumpStatus		:1;				// Состояние насоса (Вкл/Выкл)
	uint8_t	pumpPressure;					// Давление в системе подачи воды, атм. bbbb,bbbb
	uint8_t	watterLevel;					// Уровнь воды в емкости, см
} sData;	// Задание краткого имени

typedef struct {	// Структура служебных флагов
	uint8_t	tranMessageOK	:1;				// Флаг завершения передачи сообщения
	uint8_t	recMessageOK	:1;				// Флаг завершения приема сообщения
	uint8_t	recMessageNOK	:1;				// Флаг завершения приема сообщения с ошибками
	uint8_t recMessagePRE	:1;				// Флаг предыдущего завершения приема (1-OK, 2-NOK)
	uint8_t manON			:1;				// Ручная команда на ВКЛ
	uint8_t manOFF			:1;				// Ручная команда на ВЫКЛ
} sFlag;

sCommand com;								// Структура отправляемых команд
sData data;									// Структура принимаемых данных
volatile sFlag flag;						// Служебные флаги


volatile uint8_t	num = 1;				// Номер принятого/отправленного байта по UART
volatile uint8_t	tran_byte[3];			// Массив отправляемых байт. [0] - не используется
volatile uint8_t	rec_byte[5];			// Массив принимаемых байт. [0] - не используется

ISR (USART_UDRE_vect) {						// Функция передачи байта по UART через прерывание
	UDR = tran_byte[num];					// Отправить байт
	if (num == 3) {
		num = 1;
		UCSRB &= ~(1<<UDRIE);				// ВЫКЛ прерывание по освобождению регистра данных
		UCSRB |= 1<<TXCIE;					// ВКЛ прерывание по завершению передачи
	}	else num++;
}

ISR (USART_TXC_vect) {
	flag.tranMessageOK = 1;					// Сообщение успешно отправлено
	UCSRB &= ~(1<<TXCIE);					// ВЫКЛ прерывание по завершению передачи
	PORTD &= ~(1<<4);						// MAX485 на прием
	UCSRB |= 1<<RXCIE;						// ВКЛ прерывание по завершению приема
}

ISR (USART_RXC_vect) {						// Функция приема байта по UART через прерывание
	rec_byte[num] = UDR;					// Считать данные
	if (num == 1 && rec_byte[1] != 0x7E) return;
	if (num == 5) {
		num = 1;
		UCSRB &= ~(1<<RXCIE);				// ВЫКЛ прерывание по приему
		// Считаем и проверяем контрольную сумму
		uint8_t crc8 = 0xFF;
		for (uint8_t i=1; i<=4; i++) {
			crc8 = _crc8_ccitt_update(crc8, rec_byte[i]);
		}
		if(crc8 == rec_byte[5])
				flag.recMessageOK = 1;		// Сообщение успешно принято
		else	flag.recMessageNOK = 1;		// Сообщение принято с ошибками
	}
	else num++;
}


inline void startInformationExchange() {
	PORTD |= 1<<4;							// MAX485 на передачу
	UCSRB = 1<<UDRIE;						// Включить прерывания по освобождению регистра данных (для передачи)
}

ISR (INT0_vect) {							// Ручная команда ВКЛ
	flag.manON = 1;
	flag.manOFF = 0;
	_delay_us(200);
}

ISR (INT1_vect) {							// Ручная команда ВЫКЛ
	flag.manON = 0;
	flag.manOFF = 1;
	_delay_us(200);
}

void ports_init();
void tran_message();
void rec_message();

void(*print)(const uint8_t) = HD44780_print;
void(*print_adr)(const uint8_t, uint8_t) = HD44780_print_adr;
void(*printArray_adr)(const uint8_t*, const uint8_t, uint8_t) = HD44780_printArray_adr;
void(*printString_adr)(char *, uint8_t) = HD44780_printString_adr;
void(*printArray[])(const uint8_t*, const uint8_t) = {HD44780_printArray, HD44780_printArray1, HD44780_printArray2};
void(*printString[])(char*) = {HD44780_printString, HD44780_printString1, HD44780_printString2};


#endif /* PU_H_ */