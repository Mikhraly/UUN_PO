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
	timer1_init();
	
	printString[1]("������ ������...");		// ����� ���������� ������ �� �������
	
	asm("sei");								// ��������� ���������� ����������
	encryptionTranMessage();
	startInformationExchange();				// ������ �������������� �����. ��������/����� ������ (���� ����)
	
	
    while (1) 
    {	
		
		if (flag.recMessageOK || flag.recMessageNOK) {
			myCounters.connectionNOK = 0;
			
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
				if (flag.myError) {
					flag.myError = 0;
					printString[1]("�������"); print(0xA0); printString[0]("��������");	// ����� ���������
					printString[2]("    %  "); print(0xA0); printString[0]("    ���."); // ����� ������ ���������	
				}
				setAddress(0x41); printArray[0](level, 3);
				setAddress(0x49); printArray[0](pressure, 3);
				flag.recMessagePRE = 1;
			}
			
			if (flag.recMessageNOK) {			// ���� ����� �������� � ��������
				flag.recMessageNOK = 0;			// �������� ���� ������ � ��������
				flag.recMessagePRE = 0;
			}
			
			// ��������� ������ ��� ��������
			if (PINA & 1<<0) {		// �������������� �����
				if (data.watterLevel<25 && data.pumpStatus!=1) com.pumpON = 1; else com.pumpON = 0;
				if (data.watterLevel>95 && data.pumpStatus==1) com.pumpOFF = 1; else com.pumpOFF = 0;
			} else {				// ������ �����
				if (data.watterLevel == 100) {
					flag.manON = 0;
					flag.manOFF = 1;
				}
				if (flag.manON && !data.pumpStatus) com.pumpON = 1; else com.pumpON = 0;
				if (flag.manOFF && data.pumpStatus) com.pumpOFF = 1; else com.pumpOFF = 0;
			}
			encryptionTranMessage();
			startInformationExchange();
			
		}
		
		
		if (data.pumpStatus) PORTD |= 1<<5;	else PORTD &= ~(1<<5);
		
		if ( (data.pumpStatus && data.watterPressure < 0x20) || (!data.pumpStatus && data.watterPressure > 0x19) ) {
			TIMSK &= ~(1<<OCIE1B);
			PORTD |= 1<<6;
		} else {
			if ((data.watterLevel < 25) && (!data.pumpStatus))
				TIMSK |= 1<<OCIE1B;
			else {
				TIMSK &= ~(1<<OCIE1B);
				PORTD &= ~(1<<6);
			}
		}
		
		
		if (myCounters.notOn > 5) myError(0x01);
		if (myCounters.notPress > 5) myError(0x02);
		if (myCounters.notOff > 5) myError(0x03);
		if (myCounters.press > 5) myError(0x04);
		if (myCounters.messageNOK > 5) myError(0x05);
		if (myCounters.connectionNOK > 6) myError(0x06);

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
	MCUCR	= 0b00001010;			// ����� ������ ������� ���������� INT0, INT1 �� ������� ������	
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

void myError(uint8_t errorCode) {
	PORTD |= 1<<7;
	printErrorCode(errorCode);
	
	if (errorCode != 0x06) {
		com.pumpOFF = 1;
		while (!flag.recMessageOK && !flag.recMessageNOK);
		flag.recMessageOK = 0; flag.recMessageNOK = 0;
		encryptionTranMessage();
		startInformationExchange();
		while (!flag.recMessageOK && !flag.recMessageNOK);
		if (flag.recMessageOK) {
			flag.recMessageOK = 0;
			flag.recMessagePRE = 1;
			decryptionRecMessage();
			if (data.pumpStatus) PORTD |= 1<<5;	else PORTD &= ~(1<<5);
			if ( (data.pumpStatus && data.watterPressure < 0x20) || (!data.pumpStatus && data.watterPressure > 0x19) )
				PORTD |= 1<<6; else PORTD &= ~(1<<6);
		}
		com.pumpOFF = 0;
	}
	
	while (PINC & 1<<0);
	PORTD &= ~(1<<7);
	myCounters.notOn = 0;
	myCounters.notPress = 0;
	myCounters.notOff = 0;
	myCounters.press = 0;
	myCounters.messageNOK = 0;
	myCounters.connectionNOK = 0;
	flag.recMessageNOK = 0;
	flag.manON = 0;
	flag.manOFF = 0;
	flag.myError = 1;
	encryptionTranMessage();
	startInformationExchange();
}

void printErrorCode(uint8_t errorCode) {
	switch (errorCode) {
	case 0x01:
		printString[1]("  ������ #0x01  ");
		printString[2]("��������� ������");
		break;
	case 0x02:
		printString[1]("  ������ #0x02  ");
		printString[2]("������ �������� ");
		break;
	case 0x03:
		printString[1]("  ������ #0x03  ");
		printString[2]("���������� �����");
		break;
	case 0x04:
		printString[1]("  ������ #0x04  ");
		printString[2]("������� ��������");
		break;
	case 0x05:
		printString[1]("  ������ #0x05  ");
		printString[2]("����� � ��������");
		break;
	case 0x06:
		printString[1]("  ������ #0x06  ");
		printString[2]("   ����� ���!   ");
		break;
	default:
		printString[1]("  ������ #0xFF  ");
		printString[2]("��������. ������");
		break;
	}
}