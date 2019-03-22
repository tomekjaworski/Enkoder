#if !defined(_DEFS_H_)
#define _DEFS_H_

#include <p33FJ128MC706A.h>
#include <Generic.h>
#include <timer.h>
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

#define SYNC_MINOR		LATGbits.LATG2
#define SYNC_REVOLUTION	LATGbits.LATG3
// wersja dla TLP250 na glownej plytce enkoderowej
//#define SYNCOUT_HIGH	0
//#define SYNCOUT_LOW		1

// wersja dla ³¹cza róznicowego
#define SYNC_HIGH	1
#define SYNC_LOW	0


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

#define PULSES_PER_REVOLUTION_F	20000.0f
#define PULSES_PER_REVOLUTION_I	20000
#define	ROLLER_RADIUS			20.0f
#define	ROLLER_CIRCUMFERENCE	(2.0f * ROLLER_RADIUS * PI)
#define ROLLER_POS_COEF			ROLLER_CIRCUMFERENCE / (PULSES_PER_REVOLUTION_F)


// dla timera:
// F = 20MHz;
// Fcy = F/2;
// Ft1 = Fcy / 8; (preskaler);
// Tt1 = 1/Ft1;
// Tx = 50ms = 0.05s
// PR1 = Tx / Tt1;
// PR1 = 62500 (imp co 50ms dla PRE=8)
// PR1 = 1250 (imp co 1ms dla PRE=8)

#define TIMER1_50MS		62500
#define TIMER1_1MS		1250
#define TIMER1_500US	625
#define TIMER1_100US	125



union __BYTE_NIBBLES
{
	unsigned short value;
	struct
	{
		BYTE n0 : 4;
		BYTE n1 : 4;
	};	
};


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
		
	struct
	{
		BYTE b0;
		BYTE b1;
	};	
};

union __DWORD_NIBBLES
{
	unsigned long int value;
	struct
	{
		BYTE n0 : 4;
		BYTE n1 : 4;
		BYTE n2 : 4;
		BYTE n3 : 4;
		BYTE n4 : 4;
		BYTE n5 : 4;
		BYTE n6 : 4;
		BYTE n7 : 4;
	};	
};

typedef union __DWORD_NIBBLES DWORD_NIBBLES;
typedef union __WORD_NIBBLES WORD_NIBBLES;
typedef union __BYTE_NIBBLES BYTE_NIBBLES;

//#define DelayMS(x) __delay32((FCY * ( x ## UL)) / 1000UL);


#endif // _DEFS_H_


