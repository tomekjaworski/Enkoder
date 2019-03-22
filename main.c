#include <string.h>
#include <dsp.h>
#include "defs.h"
#include "lcd.h"
#include "comm.h"
#include "spi_sw.h"

// promien i srednica walca w cm,

void InitCPU(void);
BOOL TestCommand(const char* command_line, const char* pattern);

//#define _USE_LCD

struct __T1
{
	WORD counter1000ms;
	WORD counter500ms;

	BOOL recalc_velocity;
	BOOL recalc_position;
	
	WORD encoder_pos;
	WORD encoder_lastpos;
	
	float velocity;
	float position;
	const char* format_ptr;

	char str_velocity[20];	
	char str_position[20];
	
	volatile BOOL exit_run_command;
	volatile BOOL fast_position_read;
	
} T1 = {};	

struct UART
{
	BYTE timestamp;
	WORD encoder_position;
	char send_buffer[32];
	int send_pos;
	
	char recv_buffer[32];
	
	volatile int recv_pos;
	volatile int recv_plus_count;
} rs = {};

struct SYNC
{
	BOOL enabled;
	
	WORD pulse_width;	// szerokosc impulsu [100us]
	WORD delta;			// odleglosc co ile impulsow enkoderta ma byc odpalony impuls synchronizacyjny
	WORD counter;
	
	WORD sync0_counter;
	WORD sync1_counter;
	
} sync = {};	

struct AVG_WINDOW
{
	WORD position;
	WORD size;
	
	WORD data[64];
	DWORD sum;
} wnd;

void ChangeWindowSize(WORD new_size);

void Init(void);

char line[100];
DAC_COMMAND cmd;

DWORD Imax;

DWORD CNT = 0;

int main(void)
{
	BOOL first_run = TRUE;
	Init();
	
	printf("*** URUCHAMIANIE...\n");
	
	OpenSPI();
	wnd.size = 10;


	cmd.command = 0x0000;
	cmd.SHDN = 1;
	cmd.GA = 1;
	Imax = 10000; // 10,000 imp/50ms daje 600RPM przy 20,000 imp na obrót
	
	
	sync.delta = PULSES_PER_REVOLUTION_I / 8;
	sync.pulse_width = 500;

	
start:


	WriteSPI(cmd.command);

	IEC0bits.T1IE = FALSE;
	IEC1bits.U2RXIE = FALSE;
	T1.exit_run_command = FALSE;
	T1.fast_position_read = FALSE;
	rs.recv_plus_count = 0;
	sync.enabled = FALSE;

	printf("Enkoder v1.1 by TJ\n");
	printf("Ustaw parametr: 'param=wartosc' lub wpisz 'run'\n");
	printf("Zatrzymanie transmisji danych: '+++'\n\n");
	printf("Dostepne polecenia:\n");
	printf("  init\n\t- restart urzadzenia\n");
	printf("  run\n\t- uruchomienie pomiaru predkosci i pozycji\n");
	printf("  dactest\n\t- tryb testowania przetwornika D/A\n");
	printf("  fpr\n\t- Tryb FAST POSITION READ\n");
	printf("  sync\n\t- Tryb synchronizacji kamery\n");
	printf("  sync test\n\t- Tryb synchronizacji kamery (TEST)\n");
	printf("  set wndsize=xxx (1-64)\n\t- wielkosc okna do usredniania odczytow predkosci\n");
	printf("  set imax=xxxx (0-2^30)\n\t- ilosc impulsow / 50ms przy 600RPM\n");
	printf("  set dac=xxx (0-4095)\n\t- ustawienie wartosci przetrownika D/A\n");
	printf("  set spwidth=xxx (0-65e3)\n\t- dlugosc sygnalu synchronzacyjnego [x100us]\n");
	printf("  set sdelta=xxx (0-19999)\n\t- przerwa miedzykolejnymi synchronizacjami (imp. enkodera)\n");
	printf("\n");
	
	//#define CMD_EQ(__line, __cmd) (strncmp(__line, (__cmd), strlen(__cmd)) == 0)

	while(TRUE)
	{
		if (first_run)
		{
			first_run = FALSE;
			strcpy(line, "sync");
		} else
		{	
			printf("> ");
			fflush(stdout); fflush(stderr);
			gets_safe(line, 100-1, TRUE);
		}
		
		if (TestCommand(line, "init"))
			goto start;
					
		if (TestCommand(line, "run"))
			break;
			
		if (TestCommand(line, "reset"))
			asm("reset\n");
			
		if (TestCommand(line, "set~wndsize"))
		{
			printf("Aktualna wielkosc okna: %d\n", (int)wnd.size);
			WORD new_size = atoi(strchr(line, '=') + 1);
			if (new_size > 64) new_size = 64;
			if (new_size < 1) new_size = 1;
			printf("Nowa wielkosc okna: %d\nPoprzednie odczyty predkosci zostana nadpisane.\n", (int)new_size);
			ChangeWindowSize(new_size);
			continue;
		}

		if (TestCommand(line, "set~imax"))
		{
			printf("Aktualna ilosc impulsow/50ms: %ld\n", Imax);
			Imax = atol(strchr(line, '=') + 1);
			if (Imax > 0x7FFFFFFF) wnd.size = 0x7FFFFFFF;
			if (Imax == 0) Imax = 1;
			printf("Nowa ilosc impulsow/50ms: %ld\n", Imax);
			continue;
		}

		if (TestCommand(line, "dactest"))
		{		
			printf("Tryb testu przetwornika DAC.\nAby przerwac, nacisnij dowolny klawisz...\n");
			while(!KEYPRESSED)
			{
				cmd.value++;
				WriteSPI(cmd.command);
				__delay32(5000);
				LED4 = !LED4;
			}	
			getc();
			continue;
		}
		
		if (TestCommand(line, "fpr"))
		{
			printf("Rozpoczecie trybu FAST POSITION READ (binary)...\n");
			rs.send_pos = 0x00;
			PR1 = TIMER1_500US;
			T1.fast_position_read = TRUE;

			rs.send_buffer[0] = '\x3';
			CNT = 0;
			__delay32(20000000);

			IEC0bits.T1IE = TRUE;
			IEC1bits.U2RXIE = TRUE;

			while (T1.fast_position_read);

			IEC0bits.T1IE = FALSE;
			IEC1bits.U2TXIE = FALSE;
			IEC1bits.U2RXIE = FALSE;
			
			__delay32(200000);
			PR1 = TIMER1_50MS;

			printf("DONE (%d)\n", (int)CNT);
			continue;
		}	
			
		if (TestCommand(line, "set~dac"))
		{
			printf("Aktualna wartosc DAC: %d\n", cmd.value);
			cmd.value = atol(strchr(line, '=') + 1);
			WriteSPI(cmd.command);
			printf("Nowa wartosc DAC: %d (cmd=0x%04X)\n", cmd.value, cmd.command);
			continue;
		}

		if (TestCommand(line, "set~spwidth"))
		{
			printf("Aktualna dlugosc impulsu: %dx0.1ms\n", sync.pulse_width);
			sync.pulse_width = atol(strchr(line, '=') + 1);
			printf("Nowa dlugosc impulsu: %dx0.1ms\n", sync.pulse_width);
			continue;
		}		

		if (TestCommand(line, "set~sdelta"))
		{
			printf("Aktualna odleglosc miedzy impulsami synchronizacyjnymi: %d\n", sync.delta);
			sync.delta = atol(strchr(line, '=') + 1);
			printf("Nowa odleglosc miedzy impulsami synchronizacyjnymi: %d\n", sync.delta);
			continue;
		}		
		
		if (TestCommand(line, "sync~test"))
		{
			printf("Nacisnij dowolny klawisz, aby przerwac tryb testowy...\n");
			int counter = 0;
			while(!KEYPRESSED)
			{
				if (counter++ == 7)
				{
					counter = 0;
					SYNC_REVOLUTION = SYNC_HIGH;
					SYNC_MINOR = SYNC_HIGH;
					__delay_ms(sync.pulse_width / 10);
					SYNC_REVOLUTION = SYNC_LOW;
					SYNC_MINOR = SYNC_LOW;
					putc('#');
				} else
				{
					SYNC_MINOR = SYNC_HIGH;
					__delay_ms(sync.pulse_width / 10);
					SYNC_MINOR = SYNC_LOW;
					putc('.');
				}	
				__delay_ms(sync.pulse_width - sync.pulse_width / 10);			
			}	
			getc();
			continue;
		}	
		
		if (TestCommand(line, "sync"))
		{
			BYTE mode_old;
			printf("Rozpoczecie trybu synchronizacji kamery z enkoderem...\n");
			sync.counter = 0;
			sync.enabled = TRUE;
			T1.encoder_lastpos = 0;
			sync.sync0_counter = 0;
			sync.sync1_counter = 0;
			mode_old = QEICONbits.QEIM;
			QEICONbits.QEIM = 0b110;
			POSCNT = 0;
			
			PR1 = TIMER1_100US;
			
			__delay32(20000000);

			IEC0bits.T1IE = TRUE;
			IEC1bits.U2RXIE = TRUE;

			while (sync.enabled)
			{
			//	printf("%04x\n", POSCNT);
			//	__delay32(20000);
			}	

			PR1 = TIMER1_50MS;
			IEC0bits.T1IE = FALSE;
			IEC1bits.U2RXIE = FALSE;
			
			sync.sync0_counter = 0;
			sync.sync1_counter = 0;
			SYNC_MINOR = SYNC_LOW;
			SYNC_REVOLUTION = SYNC_LOW;
			
			QEICONbits.QEIM = mode_old;
			__delay32(200000);

			printf("DONE SYNC\n");
			printf("Enkoder zatrzymany na pozycji: %d\n", POSCNT);
			continue;
		}	

		printf("Nieznane polecenie: '%s'\n", line);
	}	

	T1.encoder_lastpos = POSCNT;
	rs.encoder_position = T1.encoder_lastpos;
	strcpy(rs.send_buffer, "00:0000:00000000\n");
	rs.send_pos = 0;
	
	
	PR1 = TIMER1_50MS;
	IEC0bits.T1IE = TRUE;
	IEC1bits.U2RXIE = TRUE;
	
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
			T1.recalc_velocity = FALSE;
			T1.velocity = (float)wnd.sum;
			T1.velocity /= (float)wnd.size; // srednia ilosc impulsow na 50ms
			//T1.velocity *= 3.0f / 500.0f;
			T1.velocity *= 3.0f / 50.0f; // 50 a nie 500, bo x10
			T1.format_ptr = "%.3f";

			sprintf(T1.str_velocity, T1.format_ptr, T1.velocity);
				
			LCDGotoXY(0, 0);
			LCDWriteString("V=");
			LCDWriteString(T1.str_velocity);		
			LCDWriteString("rpm  ");
		}
		
		if (T1.exit_run_command)
			goto start;	
	}	
	
	return 0;
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
//	while(TRUE)
//	{
//		LCDInit();
//	
//		LCDWriteString("Ala ma kota");
//		__delay_ms(10);
//		
//		LED3 = !LED3;
//	}
//	
//	while(TRUE)
//	{
//		
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
//	}
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

	
	//while(TRUE);
	//return 0;
//}

#define _AUTOPSV	__attribute__((auto_psv))
#define _NOAUTOPSV	__attribute__((no_auto_psv))

void _ISR _NOAUTOPSV _T1Interrupt(void) // co 50ms
{
	IFS0bits.T1IF = FALSE;
	
	WORD delta;	
	T1.encoder_pos = POSCNT;
	

	if (sync.enabled)
	{
		if (sync.sync0_counter)
		{
			sync.sync0_counter--;
			if (sync.sync0_counter == 0)
				SYNC_MINOR = SYNC_LOW;
		}	
		
		if (sync.sync1_counter)
		{
			sync.sync1_counter--;
			if (sync.sync1_counter == 0)
				SYNC_REVOLUTION = SYNC_LOW;
		}
		
		if (T1.encoder_pos >= sync.counter)
		{
			sync.sync0_counter = sync.pulse_width;
			SYNC_MINOR = SYNC_HIGH;
			sync.counter += sync.delta;
		}	
		return;
	}	
	
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

	rs.timestamp++;
	wnd.sum -= wnd.data[wnd.position];
	wnd.data[wnd.position] = delta;
	wnd.sum += delta;
	wnd.position++;
	if (wnd.position >= wnd.size)
		wnd.position = 0;


	// jesli jestesmy w trybie FPR, to nie ma potrzeby 
	// przeliczania predkosci i wyliczania napiecia wyjsciowego
	if (T1.fast_position_read)
	{
		CNT++;
		return; 
	}
	
	// wyznaczenie napiecia wyjsciowego
	DWORD Q;
	Q = wnd.sum / wnd.size;
	if (Q > Imax)
		Q = Imax;
		
	Q *= 4095;
	Q /= Imax;
	cmd.value = (WORD)Q;
	WriteSPI(cmd.command);
	

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
	
	//sync.counter = 0;
}

void _ISR _NOAUTOPSV _Interrupt58(void)//_QEIInterrupt(void)
{
	IFS3bits.QEIIF = FALSE;
	LED4 = !LED4;
	
	
	if (sync.enabled)
	{
		//SYNCOUT0 = SYNCOUT_HIGH;
		SYNC_REVOLUTION = SYNC_HIGH;
		//sync.sync0_counter = sync.pulse_width;
		sync.sync1_counter = sync.pulse_width;
		
		//sync.counter = sync.delta;
		sync.counter = 0;
	}	
}

BYTE_NIBBLES btimestamp;
WORD_NIBBLES wtemp;
DWORD_NIBBLES ltemp;

void _ISR _NOAUTOPSV _U2TXInterrupt(void)
{
	IFS1bits.U2TXIF = FALSE;
	
	if (T1.fast_position_read)
	{
		// pomiar pozycji
		btimestamp.value = rs.timestamp;
		wtemp.value = T1.encoder_pos;

		rs.send_buffer[1] = rs.timestamp;
		rs.send_buffer[2] = wtemp.b0;
		rs.send_buffer[3] = wtemp.b1;
		
		U2TXREG = rs.send_buffer[rs.send_pos];
		rs.send_pos++;
		if (rs.send_pos == 1 + 1 + 2) // wyslano '\n'
		{
			rs.send_pos = 0;
			IEC1bits.U2TXIE = FALSE;
		}
	}
	else
	{	
		// pomiar dla napedu hitachi
		if (rs.send_pos == 0)
		{
			btimestamp.value = rs.timestamp;
			wtemp.value = T1.encoder_pos;
			ltemp.value = wnd.sum;
	
			rs.send_buffer[0] = (btimestamp.n1 >= 10) ? (btimestamp.n1 + 'A' - 10) : (btimestamp.n1 + '0');
			rs.send_buffer[1] = (btimestamp.n0 >= 10) ? (btimestamp.n0 + 'A' - 10) : (btimestamp.n0 + '0');
			rs.send_buffer[2] = ':';
			rs.send_buffer[3] = (wtemp.n3 >= 10) ? (wtemp.n3 + 'A' - 10) : (wtemp.n3 + '0');
			rs.send_buffer[4] = (wtemp.n2 >= 10) ? (wtemp.n2 + 'A' - 10) : (wtemp.n2 + '0');
			rs.send_buffer[5] = (wtemp.n1 >= 10) ? (wtemp.n1 + 'A' - 10) : (wtemp.n1 + '0');
			rs.send_buffer[6] = (wtemp.n0 >= 10) ? (wtemp.n0 + 'A' - 10) : (wtemp.n0 + '0');
			rs.send_buffer[7] = ':';
			rs.send_buffer[8] = (ltemp.n7 >= 10) ? (ltemp.n7 + 'A' - 10) : (ltemp.n7 + '0');
			rs.send_buffer[9] = (ltemp.n6 >= 10) ? (ltemp.n6 + 'A' - 10) : (ltemp.n6 + '0');
			rs.send_buffer[10] = (ltemp.n5 >= 10) ? (ltemp.n5 + 'A' - 10) : (ltemp.n5 + '0');
			rs.send_buffer[11] = (ltemp.n4 >= 10) ? (ltemp.n4 + 'A' - 10) : (ltemp.n4 + '0');
			rs.send_buffer[12] = (ltemp.n3 >= 10) ? (ltemp.n3 + 'A' - 10) : (ltemp.n3 + '0');
			rs.send_buffer[13] = (ltemp.n2 >= 10) ? (ltemp.n2 + 'A' - 10) : (ltemp.n2 + '0');
			rs.send_buffer[14] = (ltemp.n1 >= 10) ? (ltemp.n1 + 'A' - 10) : (ltemp.n1 + '0');
			rs.send_buffer[15] = (ltemp.n0 >= 10) ? (ltemp.n0 + 'A' - 10) : (ltemp.n0 + '0');
			rs.send_buffer[16] = '\n';
			//rs.buffer[17] = 0;
		}
	
		U2TXREG = rs.send_buffer[rs.send_pos];
		rs.send_pos++;
		if (rs.send_buffer[rs.send_pos - 1] == '\n') // wyslano '\n'
		{
			rs.send_pos = 0;
			IEC1bits.U2TXIE = FALSE;
		}
	}		
}

void _ISR _NOAUTOPSV _U2RXInterrupt(void)
{
	unsigned char byte;
	
	IFS1bits.U2RXIF = FALSE;
	byte = U2RXREG;
	
	if (byte != '+')
	{
		rs.recv_plus_count = 0;
		return;
	}	
	rs.recv_plus_count++;
	
	if (rs.recv_plus_count != 3)
		return;
	
		
	IEC0bits.T1IE = FALSE;
	rs.recv_plus_count = 0;
	T1.exit_run_command = TRUE;
	T1.fast_position_read = FALSE;
	sync.enabled = FALSE;
	
}

void Init(void)
{
	InitCPU();

#if defined(_USE_LCD)
	LCDInit();
	LCDCursorOff();
	LCDGotoXY(2, 0);
	LCDWriteString("Enk v1.0    tj");
	LCDGotoXY(0, 1);
	LCDWriteString("Uruchamianie...");

	__delay32(20000000); //1sek
	LCDClearScreen();
	
	LCDGotoXY(0, 0);
	LCDWriteString("Oczekiwanie na\npolecenie...");
#endif
}	

void ChangeWindowSize(WORD new_size)
{
	wnd.size = new_size;
	wnd.position = 0;
	wnd.sum = 0;
	
	int i;
	for (i = 0; i < new_size; i++)
		wnd.data[i] = 0;
}	

#define toupper(_x) ((((_x) >= 'a') && ((_x) <= 'z')) ? ((_x) - 'a') + 'A' : (_x))

BOOL TestCommand(const char* command_line, const char* pattern)
{
	while(TRUE)
	{
		char p = *pattern;
		char c = *command_line;
		
		p = toupper(p);
		c = toupper(c);
		
		if (p == 0)
			return TRUE;
			
		if (p == '~')
		{
			if (c == ' ' || c == '\t')
				command_line++;
			else
				pattern++;
				
			continue;
		}		
				
				
		if (p != c)
			return FALSE;
			
		command_line++;
		pattern++;
	}	
}	