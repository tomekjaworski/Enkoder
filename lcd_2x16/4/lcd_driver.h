#ifndef _LCD_DRIVER_H
#define _LCD_DRIVER_H

#include <types.h>
#include <delays.h>

#define LCDWidth			20
#define LCDHeight			4

#define LCD_RS_PIN	LATDbits.LATD2	
#define LCD_E_PIN		LATDbits.LATD3

#define DELAY_18TCY {Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();}

//#define LCDData			if (!LATDbits.LATD2) { LATDbits.LATD2 = 1; Delay1KTCYx(2);}
//#define LCDInstruction	if (LATDbits.LATD2) { LATDbits.LATD2 = 0; Delay1KTCYx(2);}	

#define LCDData			if (!LCD_RS_PIN) { LCD_RS_PIN = 1; DELAY_18TCY;}
#define LCDInstruction	if (LCD_RS_PIN) { LCD_RS_PIN = 0; DELAY_18TCY;}	

#define LCDEnable			LCD_E_PIN = 1;
#define LCDDisable		LCD_E_PIN = 0;

//#define LCDEnable			PORTDbits.RD3 = 1;
//#define LCDDisable		PORTDbits.RD3 = 0;

extern unsigned char GlobalLCDX;
extern unsigned char GlobalLCDY;

// zapisuje bajt nie zmieniajac trybu instrukcje/dane
extern void LCDWriteByte(unsigned char byte);

// zapisuje ciag znakow w trybie dane
extern void LCDWriteStringRAM(char *str); // z pamieci RAM
extern void LCDWriteStringROM(const rom char *str); // z pamieci ROM
void LCDWriteStringROM_RPad(const rom char *str, BYTE pad_len, CHAR pad_char);


// czysci ekran i ustawia kursor w pozycji 0,0
extern void LCDClearScreen(void);
void LCDFillScreen(char znak);


// przechodzi do kolumny X i linii Y
extern void LCDGotoXY(BYTE X, BYTE Y);
extern void LCDGotoXYNoOld(BYTE X, BYTE Y); // i nie zapisuje GlobalCDx/y
// inicjuje wyswietlacz
extern void LCDInit(void);
// wygasza kursor
extern void LCDCursorOff(void);
// pokazuje kursor. block=0 - podkreslenie, block=1 - migajacy prostokat
extern void LCDCursorOn(BOOL block);
// wczytywanie odpowedniej wartosci z klawiatury
extern int LCDInputValue(	unsigned char X, unsigned char Y,
				unsigned char len,
				unsigned int min, unsigned int max,
				unsigned int *value, unsigned char lastkey);
// 10 do potegi pow
extern unsigned int power10(unsigned int pow);

// wyswietl WARTOSC w postaci DLUGOSC znakow. Puste miejsca wypelnij znakiem PAD
//extern void LCDWriteUnsignedIntPad(unsigned int wartosc, unsigned char dlugosc, unsigned char pad);
void LCDWriteUnsignedIntPad(WORD wartosc, BYTE dlugosc, CHAR pad);

// wyswietl WARTOSC w postaci DLUGOSC znakow. Puste miejsca wypelnione sa spacja (0x20)
extern void LCDWriteUnsignedInt(unsigned int wartosc, unsigned char dlugosc);
extern void LCDWriteSignedInt(signed int wartosc, unsigned char dlugosc);


void LCDSetChar5x7(BYTE char_code, rom BYTE * data);


#endif
