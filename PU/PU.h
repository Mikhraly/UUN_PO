/*
 * PU.h
 *
 * Created: 15.11.2020 15:00:40
 *  Author: MM
 */ 


#ifndef PU_H_
#define PU_H_


typedef struct {	// ��������� ������������ ������
	uint8_t	restart			:1;				// ������� �� ���������� ����� ����������
	uint8_t	pumpStatus		:1;				// ������ ��������� ������ (��� ��� ����)
	uint8_t	pumpPressure	:1;				// ������ �������� � ������� ������ ����
	uint8_t	watterLevel		:1;				// ������ ������ ���� � �������
	uint8_t	pumpON			:1;				// ������� �� ��������� ������
	uint8_t	pumpOFF			:1;				// ������� �� ���������� ������
} sCommand;	// ������� �������� �����

typedef struct {	// ��������� ����������� ������
	uint8_t	pumpStatus		:1;				// ��������� ������ (���/����)
	uint8_t	pumpPressure;					// �������� � ������� ������ ����, ���. bbbb,bbbb
	uint8_t	watterLevel;					// ������ ���� � �������, ��
} sData;	// ������� �������� �����

typedef struct {	// ��������� ��������� ������
	uint8_t	tranMessageOK	:1;				// ���� ���������� �������� ���������
	uint8_t	recMessageOK	:1;				// ���� ���������� ������ ���������
	uint8_t	recMessageNOK	:1;				// ���� ���������� ������ ��������� � ��������
	uint8_t recMessagePRE	:1;				// ���� ����������� ���������� ������ (1-OK, 2-NOK)
	uint8_t manON			:1;				// ������ ������� �� ���
	uint8_t manOFF			:1;				// ������ ������� �� ����
} sFlag;

sCommand com;								// ��������� ������������ ������
sData data;									// ��������� ����������� ������
volatile sFlag flag;						// ��������� �����


volatile uint8_t	num = 1;				// ����� ���������/������������� ����� �� UART
volatile uint8_t	tran_byte[3];			// ������ ������������ ����. [0] - �� ������������
volatile uint8_t	rec_byte[5];			// ������ ����������� ����. [0] - �� ������������

ISR (USART_UDRE_vect) {						// ������� �������� ����� �� UART ����� ����������
	UDR = tran_byte[num];					// ��������� ����
	if (num == 3) {
		num = 1;
		UCSRB &= ~(1<<UDRIE);				// ���� ���������� �� ������������ �������� ������
		UCSRB |= 1<<TXCIE;					// ��� ���������� �� ���������� ��������
	}	else num++;
}

ISR (USART_TXC_vect) {
	flag.tranMessageOK = 1;					// ��������� ������� ����������
	UCSRB &= ~(1<<TXCIE);					// ���� ���������� �� ���������� ��������
	PORTD &= ~(1<<4);						// MAX485 �� �����
	UCSRB |= 1<<RXCIE;						// ��� ���������� �� ���������� ������
}

ISR (USART_RXC_vect) {						// ������� ������ ����� �� UART ����� ����������
	rec_byte[num] = UDR;					// ������� ������
	if (num == 1 && rec_byte[1] != 0x7E) return;
	if (num == 5) {
		num = 1;
		UCSRB &= ~(1<<RXCIE);				// ���� ���������� �� ������
		// ������� � ��������� ����������� �����
		uint8_t crc8 = 0xFF;
		for (uint8_t i=1; i<=4; i++) {
			crc8 = _crc8_ccitt_update(crc8, rec_byte[i]);
		}
		if(crc8 == rec_byte[5])
				flag.recMessageOK = 1;		// ��������� ������� �������
		else	flag.recMessageNOK = 1;		// ��������� ������� � ��������
	}
	else num++;
}


inline void startInformationExchange() {
	PORTD |= 1<<4;							// MAX485 �� ��������
	UCSRB = 1<<UDRIE;						// �������� ���������� �� ������������ �������� ������ (��� ��������)
}

ISR (INT0_vect) {							// ������ ������� ���
	flag.manON = 1;
	flag.manOFF = 0;
	_delay_us(200);
}

ISR (INT1_vect) {							// ������ ������� ����
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