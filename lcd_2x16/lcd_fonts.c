#include <p18cxxx.h>
#include <types.h>
#include "lcd_fonts.h"


rom BYTE font_up_arrow[] = { // strzalka w gore
	0b00000,
	0b00100,
	0b01110,
	0b10101,
	0b00100,
	0b00100,
	0b00100,
	0b00100
	};

rom BYTE font_down_arrow[] = { // strzalka w dol
	0b00000,
	0b00100,
	0b00100,
	0b00100,
	0b00100,
	0b10101,
	0b01110,
	0b00100
	};

rom BYTE font_cross[] = { // krzyzyk
	0b00000,
	0b00000,
	0b01010,
	0b00100,
	0b01010,
	0b00000,
	0b00000,
	0b00000
	};


rom BYTE font_check[] = { // znaczek 'check'
	0b00000,
	0b00001,
	0b00010,
	0b00010,
	0b10100,
	0b10100,
	0b01000,
	0b00000
	};

rom BYTE font_full_right_arrow[] = { // zamalowana strzalka w praw¹ stronê
	0b00000,
	0b01000,
	0b01100,
	0b01110,
	0b01111,
	0b01110,
	0b01100,
	0b01000
	};

rom BYTE font_dot[] = { // kropka
	0b00000,
	0b00000,
	0b00000,
	0b00100,
	0b00000,
	0b00000,
	0b00000,
	0b00000
	};

void LCDInitFonts()
{
	//LCDSetChar5x7(0x01, font_up_arrow);
	//LCDSetChar5x7(0x02, font_down_arrow);
	LCDSetChar5x7(0x03, font_cross);
	LCDSetChar5x7(0x04, font_check);
	LCDSetChar5x7(0x05, font_full_right_arrow);
	LCDSetChar5x7(0x06, font_dot);
}

