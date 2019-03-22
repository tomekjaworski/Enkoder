/*
 * Sterownik wyswietlacza LCD 16x2
 * Autor: Tomasz Jaworski, sierpien 2006
 */

#ifndef _LCD_DRIVER_H
#define _LCD_DRIVER_H

#include <types.h>

#define LCDData			if (!PORTBbits.RB5) { PORTBbits.RB5 = 1; Delay1KTCYx(2);}
#define LCDInstruction	if (PORTBbits.RB5) { PORTBbits.RB5 = 0; Delay1KTCYx(2);}

#define LCDEnable			LATCbits.LATC4 = 1;
#define LCDDisable		LATCbits.LATC4 = 0;

extern unsigned char GlobalLCDX;
extern unsigned char GlobalLCDY;

// zapisuje bajt nie zmieniajac trybu instrukcje/dane
void LCDWriteByte(unsigned char byte);
void LCDWriteChar(char c);


// zapisuje ciag znakow w trybie dane
void LCDWriteStringRAM(char *str); // z pamieci RAM
void LCDWriteStringROM(rom char *str); // z pamieci ROM
// czysci ekran i ustawia kursor w pozycji 0,0
void LCDClearScreen(void);
// przechodzi do kolumny X i linii Y
void LCDGotoXY(unsigned char X, unsigned char Y);
void LCDGotoXYNoOld(unsigned char X, unsigned char Y); // i nie zapisuje GlobalCDx/y
// inicjuje wyswietlacz
void LCDInit(unsigned char newTRISB, unsigned char newTRISD);
// wygasza kursor
void LCDCursorOff(void);
// pokazuje kursor. block=0 - podkreslenie, block=1 - migajacy prostokat
void LCDCursorOn(unsigned char block);
// wczytywanie odpowedniej wartosci z klawiatury
int LCDInputValue(	unsigned char X, unsigned char Y,
				unsigned char len,
				unsigned int min, unsigned int max,
				unsigned int *value, unsigned char lastkey);
// 10 do potegi pow
//extern unsigned int power10(unsigned int pow);

// wyswietl WARTOSC w postaci DLUGOSC znakow. Puste miejsca wypelnij znakiem PAD
void LCDWriteUnsignedIntPad(unsigned int wartosc, unsigned char dlugosc, unsigned char pad);
// wyswietl WARTOSC w postaci DLUGOSC znakow. Puste miejsca wypelnione sa spacja (0x20)
void LCDWriteUnsignedInt(unsigned int wartosc, unsigned char dlugosc);

void LCDSetChar5x7(BYTE char_code, rom BYTE * data);

#endif
