/*
 * Sterownik wyswietlacza LCD 16x2
 * Autor: Tomasz Jaworski, sierpien 2006
 */

#include <p18cxxx.h>
#include <delays.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include "lcd_driver.h"
//#include "keyb_driver.h"

unsigned char LCDLineAddr[2] = {0x00, 0x40};

	
unsigned char GlobalLCDX, GlobalLCDY;

void LCDWriteByte(unsigned char byte)
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
	PORTC = (PORTC & 0b11110000) | ((byte >> 4) & 0b00001111);

	// impuls na magistrale
	LCDEnable;
	Delay10TCYx(3+5);	
	LCDDisable;
	Delay10TCYx(3+5);	

	// Mlodsza tetrada
//	PORTC = (PORTC & 0b11111100) | ((byte >> 2) & 0b00000011);
//	PORTB = (PORTB & 0b11001111) | ((byte << 4) & 0b00110000);
	PORTC = (PORTC & 0b11110000) | ((byte) & 0b00001111);

	// impuls na magistrale
	LCDEnable;
	Delay10TCYx(2+5);	
	LCDDisable;
	Delay10TCYx(2+5);	
	Delay10TCYx(10+5);
	
}

void LCDWriteStringRAM(char *str)
{
	LCDData;
	while (*str)
		LCDWriteByte(*str++);
}

void LCDWriteStringROM(rom char *str)
{
	LCDData;
	while (*str)
		LCDWriteByte(*str++);
}

void LCDWriteChar(char c)
{
	LCDData;
	LCDWriteByte(c);
}

void LCDClearScreen(void)
{
	LCDInstruction;
	LCDWriteByte(0b00000001); // czysc ekran
//	Delay1KTCYx(100);
	Delay1KTCYx(10);
	Delay1KTCYx(5);
	LCDWriteByte(0b10000000); // ustaw poczatek ekranu
	GlobalLCDX = 0;
	GlobalLCDY = 0;
}


void LCDGotoXY(unsigned char X, unsigned char Y)
{
	LCDInstruction;	
	LCDWriteByte(0x80 | (X + LCDLineAddr[Y]));
	GlobalLCDX = X;
	GlobalLCDY = Y;
}

void LCDGotoXYNoOld(unsigned char X, unsigned char Y)
{
	LCDInstruction;	
	LCDWriteByte(0x80 | (X + LCDLineAddr[Y]));
//	GlobalLCDX = X;
//	GlobalLCDY = Y;
}

void LCDInit(unsigned char newTRISB, unsigned char newTRISC)
{
	_asm
		nop
		nop
		nop
	_endasm
	TRISBbits.TRISB5 = 0;	// RS
	TRISCbits.TRISC4 = 0;	// E
	TRISCbits.TRISC0 = 0;	// D4
	TRISCbits.TRISC1 = 0;	// D5
	TRISCbits.TRISC2 = 0;	// D6
	TRISCbits.TRISC3 = 0;	// D7
	
//	TRISB = newTRISB & 0b11000000; // moge ustawic tylko dwa najstarsze bity jako we/wy.
//	TRISC = newTRISC & 0b00111111; // 6 mlodszych bitow sa zarezerwowane dla wyswietlacza

	// wyzerowanie szyny danych
	LATC = (PORTC &0b11100000);
	LATBbits.LATB5 = 0; // RS = 0
	//PORTD = PORTD & 0b00000011; // RD2-RD7 uzywane do LCD
	//PORTB = (PORTB & 0b11000000);
	//PORTC = (PORTC & 0b00111111);

	Delay1KTCYx(100); // > 15ms
	LCDInstruction;

	// ustawienie stanu inicjujacego
	LATCbits.LATC3 = 0;
	LATCbits.LATC2 = 0;
	LATCbits.LATC1 = 1;
	LATCbits.LATC0 = 1;

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


	LATCbits.LATC3 = 0;	// ustawienie 4 bitow na magistrali
	LATCbits.LATC2 = 0;
	LATCbits.LATC1 = 1;
	LATCbits.LATC0 = 0;

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

/*
void LCDWriteIntegerValue(unsigned int wartosc, unsigned char dlugosc)
{
	char dummy1[5]; // wyciek 1
	char tekst[6];
	char dummy2[5]; // wyciek 1
	char bufor[6]; // "65535\x0"
	dlugosc = (dlugosc>5)?5:dlugosc; // limit
	
	ltoa((unsigned long)wartosc, bufor);
	
	memset(tekst, 0x20, dlugosc);
	tekst[dlugosc] = 0;

	memmove(tekst+dlugosc-strlen(bufor), bufor, strlen(bufor));
	LCDWriteStringRAM(tekst);
}
*/
/*
void LCDWriteIntegerValue(long wartosc, unsigned char wypelnienie)
{
	// 4 294 967 295 + znak daje 1 + 1 + 3 + 3 + 3 = 11
	char tekst[16+1];
	char bufor[12];
	unsigned char len;
	
	ltoa(wartosc, bufor);
	len = strlen(bufor);
	
	if (wypelnienie > sizeof(tekst) - 1)
		wypelnienie = sizeof(tekst) - 1;
	if (wypelnienie < len)
		wypelnienie = len;

	// wpisanie spacji do bufora
	wypelnienie -= len;
	memset(tekst, 0x20, wypelnienie);
	memmove(tekst + wypelnienie, bufor, len + 1);

	LCDWriteStringRAM(tekst);

//	tekst[dlugosc] = 0;
	
	/*
	char dummy1[5]; // wyciek 1
	char tekst[6];
	char dummy2[5]; // wyciek 1
	char bufor[6]; // "65535\x0"
	dlugosc = (dlugosc>5)?5:dlugosc; // limit
	
	ltoa(wartosc, bufor);
	
	memset(tekst, 0x20, dlugosc);
	tekst[dlugosc] = 0;

	memmove(tekst+dlugosc-strlen(bufor), bufor, strlen(bufor));
	LCDWriteStringRAM(tekst);
	*/
//}


void LCDWriteUnsignedIntPad(unsigned int wartosc, unsigned char dlugosc, unsigned char pad)
{
	char tekst[6], bufor[6]; // "65535\x0"
	dlugosc = (dlugosc>5)?5:dlugosc; // limit
	
	ltoa((unsigned long)wartosc, bufor);
	
	memset(tekst, pad, dlugosc);
	tekst[dlugosc] = 0;

	memmove((void*)(tekst+dlugosc-strlen(bufor)), (void*)bufor, strlen(bufor));
	LCDWriteStringRAM(tekst);
}


void LCDCursorOn(unsigned char block)
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
/*
unsigned int power10(unsigned int pow)
{
	unsigned int res=1;	
	while (pow-- >= 1) res *= 10;
	return res;
}*/
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
