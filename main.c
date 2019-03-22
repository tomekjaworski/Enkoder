#include <string.h>
#include <dsp.h>
#include "defs.h"
#include "lcd.h"

// promien i srednica walca w cm,
#define PULSES_PER_REVOLUTION_F	20000.0f
#define PULSES_PER_REVOLUTION_I	20000
#define	ROLLER_RADIUS			30.0f
#define	ROLLER_CIRCUMFERENCE	(2.0f * ROLLER_RADIUS * PI)
#define ROLLER_POS_COEF			ROLLER_CIRCUMFERENCE / (PULSES_PER_REVOLUTION_F)

void InitCPU(void);


struct __T1
{
	WORD counter1000ms;
	WORD counter500ms;

	BOOL recalc_velocity;
	BOOL recalc_position;
	
	WORD revolution_counter;
	WORD_VAL poscnt;
	WORD poscnt_old;
	
	float velocity;
	float position;
	const char* format_ptr;

	char str_velocity[20];	
	char str_position[20];
} T1 = {};	

WORD_NIBBLES position;
WORD_NIBBLES velocity;


void Init(void);


int main(void)
{
	Init();
	MAXCNT = PULSES_PER_REVOLUTION_I - 1;
	
	SET_CPU_IPL(7);
	LCDClearScreen();
	LCDWriteString("Oczekiwanie na\npozycje walca...");
	while(!IFS3bits.QEIIF);
	SET_CPU_IPL(0);
	LCDClearScreen();
	
	while(TRUE)
	{
		if (T1.recalc_position)
		{
			T1.recalc_position = FALSE;
			
			T1.position = (float)T1.poscnt.Val;
			T1.position *= ROLLER_POS_COEF;

			sprintf(T1.str_position, "%.1f", T1.position);
			LCDGotoXY(0, 1);
			LCDWriteString("P=");
			LCDWriteString(T1.str_position);		
			LCDWriteString("cm  ");			
		}	
		
		
		if (T1.recalc_velocity)
		{
			WORD_VAL dp;
			if (T1.poscnt.Val > T1.poscnt_old)
				dp.Val = T1.poscnt.Val - T1.poscnt_old;
			else
				if (T1.poscnt.Val < T1.poscnt_old)			
					dp.Val = T1.poscnt.Val + T1.poscnt_old;
				else
					dp.Val =  0;			
			
			T1.recalc_velocity = FALSE;
			T1.velocity = (float)dp.Val;
			T1.velocity /= PULSES_PER_REVOLUTION_F;
			T1.velocity += (float)T1.revolution_counter;
			
			if (T1.revolution_counter >= 100)
				T1.format_ptr = "%.1f";
			else
				if (T1.revolution_counter >= 10)
					T1.format_ptr = "%.2f";
				else
					T1.format_ptr = "%.3f";

			sprintf(T1.str_velocity, T1.format_ptr, T1.velocity);
				
			LCDGotoXY(0, 0);
			LCDWriteString("V=");
			LCDWriteString(T1.str_velocity);		
			LCDWriteString("rpm");

			T1.revolution_counter = 0;
			T1.poscnt_old = T1.poscnt.Val;
		}	
	}	
	
//
//	WORD max_pos = 0;
//	while(TRUE)
//	{
//		WORD w = POSCNT;
//		if (w > max_pos)
//			if ((w & 0x8000) == 0)
//				max_pos = w;
//	}	
//	
//
//
//	while(TRUE)
//	{
//	
//		LCDWriteString("Ala ma kota");
//		__delay_ms(1000);
//		
//		LED3 = !LED3;
//	}
//
//	velocity.value = 0;
//	while(TRUE)
//	{
//		position.value = POSCNT;
//		
//		tekst[0] = position.n3;
//		tekst[1] = position.n2;
//		tekst[2] = position.n1;
//		tekst[3] = position.n0;
//		tekst[4] = velocity.n3;
//		tekst[5] = velocity.n2;
//		tekst[6] = velocity.n1;
//		tekst[7] = velocity.n0;
//		
//		int i;
//		for (i = 0; i < 8; i++)
//		{
//			if (tekst[i] > 9)
//				tekst[i] += 'A' - 10;
//			else
//				tekst[i] += '0';
//		}	
//		
//		for (i = 0; i < 9; i++)
//		{
//			U2TXREG = tekst[i];;
//			while(!U2STAbits.TRMT);
//		}	
//		
//		
//		__delay32(5000000);
//		
//	}	

//	char dummy;
//	while(TRUE)
//	{
//		U2TXREG = 'U';
//		while(!U2STAbits.TRMT);
//		
//		if(U2STAbits.OERR)
//			U2STAbits.OERR = 0;
//			
//		if (U2STAbits.URXDA)
//		{
//			asm("nop");
//			asm("nop");
//			dummy = U2RXREG;
//			asm("nop");
//			asm("nop");
//		}
//		__delay_ms(100);
//	}		
//	
	while(TRUE)
	{
		LCDInit();
	
		LCDWriteString("Ala ma kota");
		__delay_ms(10);
		
		LED3 = !LED3;
	}
//	
	while(TRUE)
	{
		
		//LED3 = PORTBbits.RB3;
		//LED4 = PORTBbits.RB4;
		//LED5 = PORTBbits.RB5;
		//LED3 = POSCNT & 1;
		//LED4 = POSCNT & 2;
		//LED5 = POSCNT & 4;
//						
						
//		if (QEICONbits.CNTERR)
//		{
//			QEICONbits.CNTERR = 0;
//			LED0 = !LED0;
//		}	
//		
//		LED1 = QEICONbits.INDX;
	}
//		__delay_ms(100);
//		LED0 = 0;
//		__delay_ms(100);
//		LED0 = 1;
//		__delay_ms(100);
//		LED1 = 0;
//		__delay_ms(100);
//		LED1 = 1;
//		__delay_ms(100);
//		LED2 = 0;
//		__delay_ms(100);
//		LED2 = 1;
//	}	

	
	while(TRUE);
	return 0;
}

#define _AUTOPSV	__attribute__((auto_psv))
#define _NOAUTOPSV	__attribute__((no_auto_psv))


void _ISRFAST _NOAUTOPSV _T1Interrupt(void)
{
	IFS0bits.T1IF = FALSE;
	
	T1.poscnt.Val = POSCNT;
	

	if (T1.counter1000ms >= 10)
	{
		T1.counter1000ms = 0;
		T1.recalc_velocity = TRUE;
	}

	if (T1.counter500ms >= 2)
	{
		T1.counter500ms = 0;
		T1.recalc_position = TRUE;
	}
	
	T1.counter500ms++;
	T1.counter1000ms++;
	
}

void _ISRFAST _NOAUTOPSV _QEIInterrupt(void)
{
	IFS3bits.QEIIF = FALSE;
	T1.revolution_counter++;
	T1.poscnt_old = 0;
	//LED4 = !LED4;
}


void Init(void)
{
	InitCPU();

	LCDInit();
	LCDCursorOff();
	LCDGotoXY(2, 0);
	LCDWriteString("Enk v1.0    tj");
	LCDGotoXY(0, 1);
	LCDWriteString("Uruchamianie...");

	__delay32(20000000); //1sek
	LCDClearScreen();
}	


