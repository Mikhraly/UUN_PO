/*
 * PU.c
 * Fclk=4MHz
 * Created: 08.11.2020 17:42:49
 * Author : MM
 */ 

#include "PU.h"


int main(void)
{	
	ports_init();
	hd44780_init();
    uart_init();
	
	printString[0]("������ ������...");		// ����� ���������� ������ �� �������
	
	asm("sei");								// ��������� ���������� ����������
	encryptionTranMessage();
	startInformationExchange();				// ������ �������������� �����. ��������/����� ������ (���� ����)
	
	
    while (1) 
    {	
		if (flag.recMessageOK || flag.recMessageNOK) {
			
			if (flag.recMessageOK) {
				flag.recMessageOK = 0;
				decryptionRecMessage();
			
				// ��������� ������ ��� ������ �� �������			
				uint8_t level[] = {	(data.watterLevel/100) ? (data.watterLevel/100)+48 : (uint8_t)' ',
									(data.watterLevel/10) ? ((data.watterLevel/10)%10)+48 : (uint8_t)' ',
									(data.watterLevel%10)+48 };
				uint8_t pressure[] = {	(data.watterPressure>>4)+48, (uint8_t)',',
										(data.watterPressure & 0x0F)+48 };
									
				// ����� ������ �� �������
				if (!flag.recMessagePRE) {		// ���� ���������� ��������� ������� � ��������
					printString[1]("�������"); print(0xFF); printString[0]("��������");	// ����� ���������
					printString[2]("    %  "); print(0xFF); printString[0]("    ���."); // ����� ������ ���������	
				}
				setAddress(0x41); printArray[0](level, 3);
				setAddress(0x49); printArray[0](pressure, 3);
				flag.recMessagePRE = 1;			// ���������� ��� ���������� ���������
			}
			
			if (flag.recMessageNOK) {			// ���� ����� �������� � ��������
				flag.recMessageNOK = 0;			// �������� ���� ������ � ��������
				// ������� �� ������� ��������������
				if (flag.recMessagePRE)
				printString[1]("    ��������    ");
				printString[2](" �������� ����� ");
				flag.recMessagePRE = 0;			// ���������� ��� ���������� ���������
			}
			
			if (data.pumpStatus) PORTD |= 1<<5;	else PORTD &= ~(1<<5);
			// ��������� ������ ��� ��������
			if (PINA & 1<<0) {					// �������������� �����
				if (data.watterLevel<30 && data.pumpStatus!=1)	com.pumpON = 1;		else com.pumpON = 0;
				if (data.watterLevel>90 && data.pumpStatus==1)	com.pumpOFF = 1;	else com.pumpOFF = 0;
			} else {							// ������ �����
				if (flag.manON && data.pumpStatus!=1)	com.pumpON = 1;		else com.pumpON = 0;
				if (flag.manOFF && data.pumpStatus==1)	com.pumpOFF = 1;	else com.pumpOFF = 0;
			}
			encryptionTranMessage();
			startInformationExchange();
			
		}
		
		
		// !!! 1 �������� ��������� ��� ��� ���� (����, ������� ���������� � ������)
		// !!! 2 ����������� ������ ����
		
		// ���� ��������� ����������� � �������� �� ���������� 2 ���., �� ������
		// ...���� ����� ��� ����� 2 ���., �� ������
		// ...���� ������� �� ���, � ������ ���� ����� 10 ���., �� ���� � � ������
		// ...���� ����� ������� � �������� ��� ����� 30 ���., �� ���� � ������
		// ...���� ������� �� ����, � ������ ��� ����� 10 ���., �� ���� � � ������
		
    }
}




/************************************************************************/
/*                   �������� ������� ���������                         */
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
	//MCUSR	= 0b00001010;			// ����� ������ ������� ���������� INT0, INT1 �� ������� ������	
	GICR	|= 1<<INT0 | 1<<INT1;	// �������� ������� ���������� INT0, INT1
}

void encryptionTranMessage() {	// ������������ ������������� ���������
	tran_byte[1] = 0x7E;								// ������ ���� - ���������
	tran_byte[2] =	com.pumpStatus<<4 | com.watterPressure<<3 | com.watterLevel<<2 |
					com.pumpOFF<<1 | com.pumpON<<0;		// ������ ���� - ����� ������
	// ������� ����������� �����
	uint8_t crc8 = 0xFF;
	for (uint8_t i=1; i<=2; i++) {
		crc8 = _crc8_ccitt_update(crc8, tran_byte[i]);
	}
	tran_byte[3] = crc8;								// ������ ���� - ����������� �����
}

void decryptionRecMessage() {	// ����������� ��������� ���������
	data.pumpStatus = (rec_byte[2] & 1<<7)? 1 : 0;		// ��������� ������ (���/����)
	data.watterPressure = rec_byte[3];					// �������� ������ � ���. bbbb,bbbb
	data.watterLevel = rec_byte[4];						// ������� ���� � �� (���������� �� ������� �� �����������)
}

void startInformationExchange() {
	PORTD |= 1<<4;							// MAX485 �� ��������
	_delay_us(200);
	UCSRB |= 1<<UDRIE;						// �������� ���������� �� ������������ �������� ������ (��� ��������)
}