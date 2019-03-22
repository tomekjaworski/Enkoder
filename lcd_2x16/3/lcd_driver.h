#ifndef _LCD_DRIVER_H
#define _LCD_DRIVER_H

#define	LCDData			if (!PORTCbits.RC6) { PORTCbits.RC6 = 1; Delay1KTCYx(2);}
#define LCDInstruction	if (PORTCbits.RC6) { PORTCbits.RC6 = 0; Delay1KTCYx(2);}

#define LCDEnable		PORTCbits.RC7 = 1;
#define LCDDisable		PORTCbits.RC7 = 0;

extern unsigned char GlobalLCDX;
extern unsigned char GlobalLCDY;

// zapisuje bajt nie zmieniajac trybu instrukcje/dane
extern void LCDWriteByte(unsigned char byte);
// zapisuje ciag znakow w trybie dane
extern void LCDWriteStringRAM(char *str); // z pamieci RAM
extern void LCDWriteStringROM(rom char *str); // z pamieci ROM
// czysci ekran i ustawia kursor w pozycji 0,0
extern void LCDClearScreen(void);
// przechodzi do kolumny X i linii Y
extern void LCDGotoXY(unsigned char X, unsigned char Y);
extern void LCDGotoXYNoOld(unsigned char X, unsigned char Y); // i nie zapisuje GlobalCDx/y
// inicjuje wyswietlacz
extern void LCDInit(unsigned char newTRISB, unsigned char newTRISC);
// wygasza kursor
extern void LCDCursorOff(void);
// pokazuje kursor. block=0 - podkreslenie, block=1 - migajacy prostokat
extern void LCDCursorOn(unsigned char block);

/*
// wczytywanie odpowedniej wartosci z klawiatury
extern int LCDInputValue(	unsigned char X, unsigned char Y,
				unsigned char len,
				unsigned int min, unsigned int max,
				unsigned int *value, unsigned char lastkey);
				
*/				
// 10 do potegi pow
extern unsigned int power10(unsigned int pow);


// wyswietl WARTOSC w postaci DLUGOSC znakow. Puste miejsca wypelnij znakiem PAD
extern void LCDWriteUnsignedIntPad(unsigned int wartosc, unsigned char dlugosc, unsigned char pad);
// wyswietl WARTOSC w postaci DLUGOSC znakow. Puste miejsca wypelnione sa spacja (0x20)
extern void LCDWriteUnsignedInt(unsigned int wartosc, unsigned char dlugosc);


#endif
