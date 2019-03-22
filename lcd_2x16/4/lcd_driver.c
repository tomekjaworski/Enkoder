#include <p18cxxx.h>
#include <delays.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

#include "..\lcd_2x16\lcd_driver.h"

unsigned char GlobalLCDX, GlobalLCDY;

// ###########################################################################
// ###########################################################################
// ####
// #### Kod ZALE¯NY od implementacji sprzêtowej !!
// ####
// ###########################################################################
// ###########################################################################

//unsigned char LCDLineAddr[2] = {0x00, 0x40};
unsigned char LCDLineAddr[4] = {0x00, 0x40, 0x14, 0x54};


void LCDWriteByte(BYTE byte)
{
	// 6 mlodszych bajtow jest w rejestrze RB
	// a dwa starsze (6,7) - w RD
	/*
	PORTB = (PORTB & 0b11000000) | (byte & 0b00111111);
	PORTC = (PORTC & 0b11111100) | ((byte >> 6) & 0b00000011);

	Delay10TCYx(2);	//
	LCDEnable;
	Delay10TCYx(2);	//
	LCDDisable;
	Delay10TCYx(30);//*/
	
	// dla magistrali 4 bitowej
	
	// najpierw starsza a potem mldosza tetrada

	//DB4, DB5, DB6, DB7
	//RC1, RC0, RB5, RB4
	//D7   D6   D5   D4		- starsza tatrada
	//D3   D2   D1   D0		- mlodsza tatrada


	// STARSZA TETRADA
//	PORTC = (PORTC & 0b11111100) | ((byte >> 6) & 0b00000011);
//	PORTB = (PORTB & 0b11001111) | (byte & 0b00110000);
	PORTD = (PORTD & 0b00001111) | (byte & 0b11110000);

	// impuls na magistrale
	LCDEnable;
	Delay10TCYx(3+5);	
	LCDDisable;
	Delay10TCYx(3+5);	

	// Mlodsza tetrada
//	PORTC = (PORTC & 0b11111100) | ((byte >> 2) & 0b00000011);
//	PORTB = (PORTB & 0b11001111) | ((byte << 4) & 0b00110000);
	PORTD = (PORTD & 0b00001111) | ((byte << 4) & 0b11110000);

	// impuls na magistrale
	LCDEnable;
	Delay10TCYx(2+5);	
	LCDDisable;
	Delay10TCYx(2+5);	
	Delay10TCYx(10+5);
}

/*
void LCDWriteByte(BYTE byte)
{
	// STARSZA TETRADA
	PORTD &= 0b00001111;
	PORTD |= (byte & 0b11110000);
	//PORTD = (PORTD & 0b00001111) | (byte & 0b11110000);

	//LCD_RS_PIN = 1;
	DELAY_18TCY;
	LCD_E_PIN = 1;
	DELAY_18TCY;
	LCD_E_PIN = 0;
		

	// impuls na magistrale
	//LCDEnable;
	//Delay10TCYx(3+5);	
	//LCDDisable;
	//Delay10TCYx(3+5);	

	// Mlodsza tetrada
//	PORTC = (PORTC & 0b11111100) | ((byte >> 2) & 0b00000011);
//	PORTB = (PORTB & 0b11001111) | ((byte << 4) & 0b00110000);

	PORTD &= 0b00001111;
	PORTD |= ((byte << 4) & 0b11110000);

	// impuls na magistrale
	//LCDEnable;
	//Delay10TCYx(2+5);	
	//LCDDisable;
	//Delay10TCYx(2+5);	
	//Delay10TCYx(10+5);
	
	DELAY_18TCY;
	LCD_E_PIN = 1;
	DELAY_18TCY;
	LCD_E_PIN = 0;
}
*/


void LCDInit(void)
{
	_asm
		nop
		nop
		nop
	_endasm
	TRISDbits.TRISD2 = 0;	// RS
	TRISDbits.TRISD3 = 0;	// E
	TRISDbits.TRISD4 = 0;	// D4
	TRISDbits.TRISD5 = 0;	// D5
	TRISDbits.TRISD6 = 0;	// D6
	TRISDbits.TRISD7 = 0;	// D7
	
//	TRISB = newTRISB & 0b11000000; // moge ustawic tylko dwa najstarsze bity jako we/wy.
//	TRISC = newTRISC & 0b00111111; // 6 mlodszych bitow sa zarezerwowane dla wyswietlacza

	// wyzerowanie szyny danych
	PORTD = PORTD & 0b00000011; // RD2-RD7 uzywane do LCD
	//PORTB = (PORTB & 0b11000000);
	//PORTC = (PORTC & 0b00111111);

	Delay1KTCYx(100); // > 15ms
	LCDInstruction;

	// ustawienie stanu inicjujacego
	PORTDbits.RD7 = 0;
	PORTDbits.RD6 = 0;
	PORTDbits.RD5 = 1;
	PORTDbits.RD4 = 1;

	LCDEnable;			// pierwszy impuls
	Delay10TCYx(3);	
	LCDDisable;
	Delay10TCYx(3);	

	Delay1KTCYx(30);	// przerwa

	LCDEnable;			// drugi impuls
	Delay10TCYx(3);	
	LCDDisable;
	Delay10TCYx(3);	

	Delay1KTCYx(30);	// druga przerwa

	LCDEnable;			// trzeci impuls
	Delay10TCYx(3);	
	LCDDisable;
	Delay10TCYx(3);	

	Delay1KTCYx(30);	// trzecia przerwa


	PORTDbits.RD7 = 0;	// ustawienie 4 bitow na magistrali
	PORTDbits.RD6 = 0;
	PORTDbits.RD5 = 1;
	PORTDbits.RD4 = 0;

	LCDEnable;			// i impuls
	Delay10TCYx(3);	
	LCDDisable;
	Delay10TCYx(3);	

	Delay1KTCYx(30);	// i jeszcze jedna przerwa


	// lecimy z reszta konfiguracji

	LCDWriteByte(0b00101000); // 8 bitow, 2 linie, 5x7 pixli
	Delay1KTCYx(30);

	LCDWriteByte(0b00001000); // wylacz lcd
	Delay1KTCYx(30);

	LCDWriteByte(0b00000001); // wlacz LCD
	Delay1KTCYx(30);

	LCDWriteByte(0b00001110); // wlacz wyswietlacz, kursor widoczny
	Delay1KTCYx(30);
	LCDWriteByte(0b00000110); // zwiekszaj wskaznik, nie przesuwaj ekranu
	Delay1KTCYx(30);
	LCDClearScreen();
}


// ###########################################################################
// ###########################################################################
// ####
// #### Kod NIEZALE¯NY od implementacji sprzêtowej !!
// ####
// ###########################################################################
// ###########################################################################



void LCDWriteStringRAM(char *str)
{
	LCDData;
	while (*str)
		LCDWriteByte(*str++);
}


void LCDWriteStringROM(const rom char *str)
{
	char znak;
	LCDData;
	//while (*str)
	//	LCDWriteByte(*str++);
	
	while (znak = *str)
	{
		if (znak == '\n')
		{
			LCDGotoXY(0, GlobalLCDY + 1);
			LCDData;
			str++;
			continue;
		}
		LCDWriteByte(znak);
		str++;
	}

}

void LCDWriteStringROM_RPad(const rom char *str, BYTE pad_len, CHAR pad_char)
{
	BYTE l = 0;
	LCDData;
	while (*str)
	{
		l++;
		LCDWriteByte(*str++);
	}
	while(l++ < pad_len)
		LCDWriteByte(pad_char);
}

void LCDClearScreen(void)
{
	LCDInstruction;
	LCDWriteByte(0b00000001); // czysc ekran
	Delay1KTCYx(100);
	LCDWriteByte(0b10000000); // ustaw poczatek ekranu
	GlobalLCDX = 0;
	GlobalLCDY = 0;
}

void LCDFillScreen(char znak)
{
	BYTE i, j;
	for (i = 0; i < LCDHeight; i++)
	{
		LCDGotoXY(0, i);
		LCDData;
		for (j = 0; j < LCDWidth; j++)
			LCDWriteByte(znak);
	}	
}


void LCDGotoXY(BYTE x, BYTE y)
{
	LCDInstruction;	
	LCDWriteByte(0x80 | (x + LCDLineAddr[y]));
	GlobalLCDX = x;
	GlobalLCDY = y;
}

void LCDGotoXYNoOld(BYTE x, BYTE y)
{
	LCDInstruction;	
	LCDWriteByte(0x80 | (x + LCDLineAddr[y]));
//	GlobalLCDX = x;
//	GlobalLCDY = y;
}


void LCDWriteUnsignedInt(unsigned int wartosc, unsigned char dlugosc)
{
	char dummy1[5]; // wyciek 1
	char tekst[6];
	char dummy2[5]; // wyciek 1
	char bufor[6]; // "65535\x0"
	dlugosc = (dlugosc>5)?5:dlugosc; // limit
	
	ltoa((unsigned long)wartosc, bufor);
	
	memset(tekst, 0x20, dlugosc);
	tekst[dlugosc] = 0;

	memmove(tekst+dlugosc-strlen(bufor), (void*)bufor, strlen(bufor));
	LCDWriteStringRAM(tekst);
}


void LCDWriteSignedInt(signed int wartosc, unsigned char dlugosc)
{
	char dummy1[5]; // wyciek 1
	char tekst[6];
	char dummy2[5]; // wyciek 1
	char bufor[6]; // "65535\x0"
	dlugosc = (dlugosc>5)?5:dlugosc; // limit
	
	ltoa((long)wartosc, bufor);
	
	memset(tekst, 0x20, dlugosc);
	tekst[dlugosc] = 0;

	memmove(tekst+dlugosc-strlen(bufor), (void*)bufor, strlen(bufor));
	LCDWriteStringRAM(tekst);
}
/*
void LCDWriteUnsignedIntPad(unsigned int wartosc, unsigned char dlugosc, unsigned char pad)
{
	char tekst[6], bufor[6]; // "65535\x0"
	dlugosc = (dlugosc>5)?5:dlugosc; // limit
	
	ltoa((unsigned long)wartosc, bufor);
	
	memset(tekst, pad, dlugosc);
	tekst[dlugosc] = 0;

	memmove(tekst+dlugosc-strlen(bufor), (void*)bufor, strlen(bufor));
	LCDWriteStringRAM(tekst);
}
*/

char  __bufor[6];
BYTE __len, __len2;
void LCDWriteUnsignedIntPad(WORD wartosc, BYTE dlugosc, CHAR pad)
{
	//char tekst[6], bufor[6]; // "65535\x0"
	dlugosc = (dlugosc > 5) ? 5 : dlugosc; // limit
	
	ultoa(wartosc, __bufor);
	__len = strlen(__bufor);
	
	LCDData;
	__len2 = dlugosc - __len;
	while(__len2--)
		LCDWriteByte(pad);
	__len2 = 0x00;
	while(__len--)
		LCDWriteByte(__bufor[__len2++]);
}


void LCDCursorOn(BOOL block)
{
	LCDInstruction;	
	if (block)
		LCDWriteByte(0b00001111);
	else
		LCDWriteByte(0b00001110);
}

void LCDCursorOff(void)
{
	LCDInstruction;	
	LCDWriteByte(0b00001100);
}

unsigned int power10(unsigned int pow)
{
	unsigned int res=1;	
	while (pow-- >= 1) res *= 10;
	return res;
}
/*
int LCDInputValue(	unsigned char X, unsigned char Y,
				unsigned char len,
				unsigned int min, unsigned int max,
				unsigned int *value, unsigned char lastkey)
{
	unsigned char pozycja = 0, key;
	long wart = *value;
	unsigned char StanPrzerwan = INTCONbits.GIE;

	// gdzie zaczynamy?
	pozycja = (lastkey == KB_RIGHT)?0:(len-1);
	key = KB_NONE;
	while (key != KB_EXITCONDITION)
	{
		// ograniczniki
		wart = (wart > max)?max:wart;
		wart = (wart < min)?min:wart;
		// wyswietlenie wartosci
		INTCONbits.GIE = 0;
		LCDCursorOff();
		LCDGotoXY(X, Y);
		LCDWriteUnsignedIntPad(wart, len, '0');

		// wyswietlenie kursora
		LCDGotoXY(X+pozycja, Y);
		LCDCursorOn(0); 

		INTCONbits.GIE = StanPrzerwan;

		key = GetKeySync(1);
		if (key == KB_LEFT) // czy strzalka w lewo ?
		{
			if (pozycja == 0) break;
			pozycja--; // a jak nie to zmniejsz pozycje
		}
		if (key == KB_RIGHT) // czy strzalka w prawo ??
		{
			if (pozycja == len-1) break;
			pozycja++; // nastepna pozycja (zwieksz pozycje)
		}
		if (key == KB_UP) // zwieksz odpowiednie miejsce
			wart += power10(len-pozycja-1);
		if (key == KB_DOWN) // zwieksz odpowiednie miejsce
			wart -= power10(len-pozycja-1);
	}
	*value = (unsigned int)wart;
	return key;
}

*/

void LCDSetChar5x7(BYTE char_code, rom BYTE * data)
{
	BYTE i, row;
	
	char_code = char_code << 3; // bo 3 pierwsze bity zajmuje adres linii znaku
	for (i = 0; i < 8; i++)
	{
		row = data[i] & 0b00011111; // maska nakladana na czcionke
		
		LCDInstruction;	
		LCDWriteByte(0x40 | char_code | i); // okreslenie pozycji linii w zadanym znaku
		
		LCDData;
		LCDWriteByte(row);
	}
}
