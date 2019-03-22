#include <p18cxxx.h>
#include <delays.h>
#include <stdlib.h>
#include <string.h>
#include "lcd_driver.h"
//#include "keyb_driver.h"

unsigned char LCDLineAddr[2] = {0x00, 0x40};
	
unsigned char GlobalLCDX, GlobalLCDY;

void LCDWriteByte(unsigned char byte)
{
	//DB4, DB5, DB6, DB7
	//RC1, RC0, RB5, RB4
	//D7   D6   D5   D4		- starsza tatrada
	//D3   D2   D1   D0		- mlodsza tatrada

	// STARSZA TETRADA
	PORTC = (PORTC & 0b11111100) | ((byte >> 6) & 0b00000011);
	PORTB = (PORTB & 0b11001111) | (byte & 0b00110000);

	// impuls na magistrale
	LCDEnable;
	Delay10TCYx(5/*3+30*/);
	LCDDisable;
	Delay10TCYx(5/*3+30*/);

	PORTC = (PORTC & 0b11111100) | ((byte >> 2) & 0b00000011);
	PORTB = (PORTB & 0b11001111) | ((byte << 4) & 0b00110000);

	// impuls na magistrale
	LCDEnable;
	Delay10TCYx(5/*2+30*/);
	LCDDisable;
	Delay10TCYx(5/*2+30*/);
	Delay10TCYx(15/*10+30*/);
	
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

void LCDClearScreen(void)
{
	LCDInstruction;
	LCDWriteByte(0b00000001); // czysc ekran
	Delay1KTCYx(100);
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
	TRISB = newTRISB & 0b11000000; // moge ustawic tylko dwa najstarsze bity jako we/wy.
	TRISC = newTRISC & 0b00111111; // 6 mlodszych bitow sa zarezerwowane dla wyswietlacza

	// wyzerowanie szyny danych
	PORTB = (PORTB & 0b11000000);
	PORTC = (PORTC & 0b00111111);

	Delay1KTCYx(100); // > 15ms
	LCDInstruction;

	// ustawienie stanu inicjujacego
	PORTCbits.RC1 = 0;
	PORTCbits.RC0 = 0;
	PORTBbits.RB5 = 1;
	PORTBbits.RB4 = 1;

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


	PORTCbits.RC1 = 0;	// ustawienie 4 bitow na magistrali
	PORTCbits.RC0 = 0;
	PORTBbits.RB5 = 1;
	PORTBbits.RB4 = 0;

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

	memmove(tekst+dlugosc-strlen(bufor), bufor, strlen(bufor));
	LCDWriteStringRAM(tekst);
}

void LCDWriteUnsignedIntPad(unsigned int wartosc, unsigned char dlugosc, unsigned char pad)
{
	char tekst[6], bufor[6]; // "65535\x0"
	dlugosc = (dlugosc>5)?5:dlugosc; // limit
	
	ltoa((unsigned long)wartosc, bufor);
	
	memset(tekst, pad, dlugosc);
	tekst[dlugosc] = 0;

	memmove(tekst+dlugosc-strlen(bufor), bufor, strlen(bufor));
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
