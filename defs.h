#if !defined(_DEFS_H_)
#define _DEFS_H_

#include <p33FJ128MC706A.h>
#include <Generic.h>
#include <timer.h>
#include <generic.h>
#include <qei.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

// oscylator = 20MHz
#define Fosc	20000000UL

#define FCY		(Fosc/2UL)
#include <libpic30.h>

//#define TRUE	1
//#define FALSE	0
//#define NULL	(void*)0


void __delay32(DWORD cnt);

#define LED3	LATCbits.LATC14
#define LED4	LATDbits.LATD1
#define LED5	LATCbits.LATC13

#define LCD_RS	LATEbits.LATE4
#define LCD_RW	LATEbits.LATE6
#define LCD_E	LATEbits.LATE5

#define LCD_D4	LATEbits.LATE0
#define LCD_D5	LATEbits.LATE1
#define LCD_D6	LATEbits.LATE2
#define LCD_D7	LATEbits.LATE3


union __WORD_NIBBLES
{
	unsigned short value;
	struct
	{
		BYTE n0 : 4;
		BYTE n1 : 4;
		BYTE n2 : 4;
		BYTE n3 : 4;
	};	
};

typedef union __WORD_NIBBLES WORD_NIBBLES;

//#define DelayMS(x) __delay32((FCY * ( x ## UL)) / 1000UL);


#endif // _DEFS_H_


