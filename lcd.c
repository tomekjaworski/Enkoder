#include "defs.h"
#include "lcd.h"

#define Delay1KTCYx(x) __delay32((x+5)*1000UL);
#define Delay10TCYx(x) __delay32((x+5)*10UL);
#define Delay10KTCYx(x) __delay32((x+5)*10000UL);

#define LCDEnable		LCD_E = 1;
#define LCDDisable		LCD_E = 0;

#define	LCDData			if (!LCD_RS) { LCD_RS = 1; Delay1KTCYx(2);}
#define LCDInstruction	if (LCD_RS) { LCD_RS = 0; Delay1KTCYx(2);}

unsigned char LCDLineAddr[2] = {0x00, 0x40};
unsigned char GlobalLCDX, GlobalLCDY;

void LCDWriteByte(unsigned char byte)
{
	unsigned short late;
	//DB4, DB5, DB6, DB7
	//RC1, RC0, RB5, RB4
	//D7   D6   D5   D4		- starsza tatrada
	//D3   D2   D1   D0		- mlodsza tatrada

	
	// STARSZA TETRADA
	late = LATE & 0xFFF0;
	LATE = late | (byte >> 4);
//	PORTC = (PORTC & 0b11111100) | ((byte >> 6) & 0b00000011);
//	PORTB = (PORTB & 0b11001111) | (byte & 0b00110000);
//
	// impuls na magistrale
	LCDEnable;
	Delay10TCYx(5/*3+30*/);
	LCDDisable;
	Delay10TCYx(5/*3+30*/);

	late = LATE & 0xFFF0;
	LATE = late | (byte & 0x0F);
	//PORTC = (PORTC & 0b11111100) | ((byte >> 2) & 0b00000011);
	//PORTB = (PORTB & 0b11001111) | ((byte << 4) & 0b00110000);

	// impuls na magistrale
	LCDEnable;
	Delay10TCYx(5/*2+30*/);
	LCDDisable;
	Delay10TCYx(5/*2+30*/);
	Delay10TCYx(15/*10+30*/);
	
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


void LCDInit(void)
{
	asm("nop\n"
		"nop\n");
	
	// wyzerowanie szyny danych
	LCD_E = 0;
	LCD_RW = 0;
	LCD_RS = 0;
	LCD_D4 = 0;
	LCD_D5 = 0;
	LCD_D6 = 0;
	LCD_D7 = 0;

	//__delay_ms(16);
	Delay10KTCYx(150);
	LCDInstruction;

	// ustawienie stanu inicjujacego
	LCD_D7 = 0;
	LCD_D6 = 0;
	LCD_D5 = 1;
	LCD_D4 = 1;

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


	LCD_D7 = 0;
	LCD_D6 = 0;
	LCD_D5 = 1;
	LCD_D4 = 0;

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


void LCDWriteString(const char *str)
{
	char znak;
	LCDData;

	while ((znak = *str))
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



void LCDWriteChar(char c)
{
	LCDData;
	LCDWriteByte(c);
}

void LCDGotoXY(unsigned char X, unsigned char Y)
{
	LCDInstruction;	
	LCDWriteByte(0x80 | (X + LCDLineAddr[Y]));
	GlobalLCDX = X;
	GlobalLCDY = Y;
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

