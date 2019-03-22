#include <string.h>
#include <dsp.h>
#include <uart.h>
#include "defs.h"
#include "lcd.h"

// promien i srednica walca w cm,

void InitCPU(void);


struct __T1
{
	WORD counter1000ms;
	WORD counter500ms;

	BOOL recalc_velocity;
	BOOL recalc_position;
	
	WORD encoder_pos;
	WORD encoder_lastpos;
	//DWORD encoder_velocity_counter; // impulsy na sekunde
	//DWORD encoder_velocity;
	
	float velocity;
	float position;
	const char* format_ptr;

	char str_velocity[20];	
	char str_position[20];
} T1 = {};	

struct UART
{
	WORD encoder_position;
	char buffer[32];
	int pos;
} rs = {};

struct AVG_WINDOW
{
	DWORD position;
	DWORD size;
	DWORD items;
	
	WORD data[64];
	DWORD sum;
} wnd;


void Init(void);
char line[100];

#undef putc

void putc(char c)
{
	while(U2STAbits.UTXBF);
    U2TXREG = c;
}

char* gets(char* str)
{
	char* ptr = str;
	char c;
	
	while(DataRdyUART2())
		c = U2RXREG;

	while(TRUE)
	{
		while(!DataRdyUART2());
		c = U2RXREG;
		
		if ((c == '\n') || (c == '\r'))
		{
			*ptr = '\x0';
			putc('\n');
			return str;
		}
		
		putc(c);
			
		*ptr = c;
		ptr++;
	}
}

int main(void)
{
	Init();
	wnd.size = 10;


	printf("Enkoder v1.1 by TJ\n");
	printf("Ustaw parametr: 'param=wartosc' lub wpisz 'run'\n");

	while(TRUE)
	{
		printf("> ");
		fflush(stdout); fflush(stderr);
		gets(line);
		
		if (strcmp(line, "run") == 0)
			break;
			
		if (strncmp(line, "wndsize", 7) == 0)
		{
			wnd.size = atoi(strchr(line, '=') + 1);
			if (wnd.size > 64) wnd.size = 64;
			if (wnd.size < 1) wnd.size = 1;
			wnd.position = 0;
			printf("Nowa wartosc okna: %d\nPoprzednie odczyty predkosci zostana nadpisane.\n", (int)wnd.size);
			continue;
		}
		
		printf("Nieznane polecenie: '%s'\n", line);
	}	

	T1.encoder_lastpos = POSCNT;
	rs.encoder_position = T1.encoder_lastpos;
	strcpy(rs.buffer, "0000:00000000\n");
	rs.pos = 0;
	
	IEC0bits.T1IE = TRUE;
	
	//IEC1bits.U2TXIE = TRUE;
	//while(TRUE)
	//	asm("nop");
	
	LCDClearScreen();
	
	while(TRUE)
	{
		if (T1.recalc_position)
		{
			T1.recalc_position = FALSE;
			
			T1.position = (float)T1.encoder_pos;
			T1.position *= ROLLER_POS_COEF;

			sprintf(T1.str_position, "%.1f", T1.position);
			LCDGotoXY(0, 1);
			LCDWriteString("P=");
			LCDWriteString(T1.str_position);		
			LCDWriteString("cm  ");			
		}	
		
		
		if (T1.recalc_velocity)
		{
//			WORD_VAL dp;
//			if (T1.poscnt.Val > T1.poscnt_old)
//				dp.Val = T1.poscnt.Val - T1.poscnt_old;
//			else
//				if (T1.poscnt.Val < T1.poscnt_old)			
//					dp.Val = T1.poscnt.Val + T1.poscnt_old;
//				else
//					dp.Val =  0;			
			
			T1.recalc_velocity = FALSE;
			//T1.velocity = (float)T1.encoder_velocity;
			T1.velocity = (float)wnd.sum;
			T1.velocity *= 3.0f / 500.0f;
			//T1.velocity /= PULSES_PER_REVOLUTION_F;
			//T1.velocity += (float)T1.revolution_counter;
			
//			if (T1.revolution_counter >= 100)
//				T1.format_ptr = "%.1f";
//			else
//				if (T1.revolution_counter >= 10)
//					T1.format_ptr = "%.2f";
//				else
					T1.format_ptr = "%.3f";

			sprintf(T1.str_velocity, T1.format_ptr, T1.velocity);
				
			LCDGotoXY(0, 0);
			LCDWriteString("V=");
			LCDWriteString(T1.str_velocity);		
			LCDWriteString("rpm  ");

		//	T1.revolution_counter = 0;
			//T1.poscnt_old = T1.poscnt.Val;
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

void _ISRFAST _NOAUTOPSV _T1Interrupt(void) // co 50ms
{
	IFS0bits.T1IF = FALSE;
	
	WORD delta;	
	T1.encoder_pos = POSCNT;
	IEC1bits.U2TXIE = TRUE;
	
	if (T1.encoder_pos < T1.encoder_lastpos)
	{
		// nastapilo wyzerowanie licznika (sygnal INDX)
		delta = PULSES_PER_REVOLUTION_I - T1.encoder_lastpos;
		delta += T1.encoder_pos;
	} else
	{
		// od ostatniego pomiaru nie bylo sygnalu INDX
		delta = T1.encoder_pos - T1.encoder_lastpos;
	}
	T1.encoder_lastpos = T1.encoder_pos;
	
	wnd.sum -= wnd.data[wnd.position];
	wnd.data[wnd.position] = delta;
	wnd.sum += delta;
	wnd.position++;
	if (wnd.position >= wnd.size)
		wnd.position = 0;
	else
		wnd.items++;
	

	if (T1.counter1000ms >= 20)
	{
		T1.counter1000ms = 0;
		T1.recalc_velocity = TRUE;
	}

	if (T1.counter500ms >= 10)
	{
		LED3 = ~LED3;
		T1.counter500ms = 0;
		T1.recalc_position = TRUE;
		
	//	T1.encoder_velocity = T1.encoder_velocity_counter;
	//	T1.encoder_velocity_counter = delta;
	} //else
	//	T1.encoder_velocity_counter += delta;
	
	T1.counter500ms++;
	T1.counter1000ms++;
	
}

void _ISRFAST _NOAUTOPSV _QEIInterrupt(void)
{
	IFS3bits.QEIIF = FALSE;
	//T1.revolution_counter++;
	//T1.poscnt_old = 0;
	LED4 = !LED4;
}

WORD_NIBBLES u2_wtemp;
DWORD_NIBBLES u2_ltemp;

void _ISRFAST _NOAUTOPSV _U2TXInterrupt(void)
{
	IFS1bits.U2TXIF = FALSE;
	
	if (rs.pos == 0)
	{
		u2_wtemp.value = T1.encoder_pos;
		//u2_ltemp.value = T1.encoder_velocity;
		u2_ltemp.value = wnd.sum;
		
		rs.buffer[0] = (u2_wtemp.n3 >= 10) ? (u2_wtemp.n3 + 'A' - 10) : (u2_wtemp.n3 + '0');
		rs.buffer[1] = (u2_wtemp.n2 >= 10) ? (u2_wtemp.n2 + 'A' - 10) : (u2_wtemp.n2 + '0');
		rs.buffer[2] = (u2_wtemp.n1 >= 10) ? (u2_wtemp.n1 + 'A' - 10) : (u2_wtemp.n1 + '0');
		rs.buffer[3] = (u2_wtemp.n0 >= 10) ? (u2_wtemp.n0 + 'A' - 10) : (u2_wtemp.n0 + '0');
		rs.buffer[4] = ':';
		rs.buffer[5] = (u2_ltemp.n7 >= 10) ? (u2_ltemp.n7 + 'A' - 10) : (u2_ltemp.n7 + '0');
		rs.buffer[6] = (u2_ltemp.n6 >= 10) ? (u2_ltemp.n6 + 'A' - 10) : (u2_ltemp.n6 + '0');
		rs.buffer[7] = (u2_ltemp.n5 >= 10) ? (u2_ltemp.n5 + 'A' - 10) : (u2_ltemp.n5 + '0');
		rs.buffer[8] = (u2_ltemp.n4 >= 10) ? (u2_ltemp.n4 + 'A' - 10) : (u2_ltemp.n4 + '0');
		rs.buffer[9] = (u2_ltemp.n3 >= 10) ? (u2_ltemp.n3 + 'A' - 10) : (u2_ltemp.n3 + '0');
		rs.buffer[10] = (u2_ltemp.n2 >= 10) ? (u2_ltemp.n2 + 'A' - 10) : (u2_ltemp.n2 + '0');
		rs.buffer[11] = (u2_ltemp.n1 >= 10) ? (u2_ltemp.n1 + 'A' - 10) : (u2_ltemp.n1 + '0');
		rs.buffer[12] = (u2_ltemp.n0 >= 10) ? (u2_ltemp.n0 + 'A' - 10) : (u2_ltemp.n0 + '0');
		rs.buffer[13] = '\n';
	}

	U2TXREG = rs.buffer[rs.pos];
	rs.pos++;
	if (rs.pos == 14) // wyslano '\n'
	{
		rs.pos = 0;
		IEC1bits.U2TXIE = FALSE;
	}
	
}

void _ISRFAST _NOAUTOPSV _U2RXInterrupt(void)
{
	unsigned char byte;
	
	IFS1bits.U2RXIF = FALSE;
	byte = U2RXREG;
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


