#include <string.h>
#include <dsp.h>
#include "defs.h"
//#include "lcd.h"
#include "comm.h"
#include "spi_sw.h"

WORD CalcCRC16(const char* buffer, WORD cnt);	// w misc.c
WORD String2Hex16(const char* str);				// w misc.c
int isXdigit(char x);
WORD String2WORD(const char* str);

void SetValve(int valve, int state);

//						RD2			RD3			RD4			RD5		RD6			RD7
BYTE power_bits[] = {0b00000100,0b00001000,0b00010000,0b00100000,0b01000000,0b10000000};

#define RECV_BUFFER_SIZE	200
#define ANGLES_PER_CHANNEL	16


// promien i srednica walca w cm,

void InitCPU(void);
BOOL TestCommand(const char* command_line, const char* pattern);

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

#define POWER_CASE_A	0x11
#define POWER_CASE_B	0x12
#define POWER_FIXED_OFF	0x00
#define POWER_FIXED_ON	0x01

struct POWER_ANGLE
{
	BYTE mode;
	WORD start;
	WORD stop;
};

struct POWER_CHANNEL
{
	struct POWER_ANGLE *pointer, *end;
	struct POWER_ANGLE current_angle;
	struct POWER_ANGLE angles[ANGLES_PER_CHANNEL];
	WORD count;
	BOOL reload;
	BOOL power_on;
} channels[6];

struct SYNC
{
	BOOL enabled;
	
	WORD pulse_width;	// szerokosc impulsu [100us]
	WORD delta;			// odleglosc co ile impulsow enkoderta ma byc odpalony impuls synchronizacyjny
	WORD pulses_per_revolution; // liczba impulsow na obrot
	WORD counter;
	
	WORD sync0_counter;
	WORD sync1_counter;

	struct POWER_ANGLE angles_new[ANGLES_PER_CHANNEL];
	
//	struct POWER_ANGLE power[6];
//	struct POWER_ANGLE power_new[6];
	BOOL angles_ready;
//	BOOL power_new_ready;
	
	char buffer[RECV_BUFFER_SIZE];
	WORD buffer_pos;
	BOOL data_received;
	
} sync = {};	

struct WATER_VALVES {
	WORD mark_counter;

	WORD counter500ms;
	WORD timer_500ms;
	
	WORD valve_off_delay;
	WORD valve_off_timer;
	
} water = {};




struct AVG_WINDOW
{
	WORD position;
	WORD size;
	
	WORD data[64];
	DWORD sum;
} wnd;

void ChangeWindowSize(WORD new_size);

char line[100];
DAC_COMMAND cmd;
DWORD Imax;
DWORD CNT = 0;
BOOL autostart_sync;
int main(void)
{
	InitCPU();
	
#if defined(__DEBUG)
	autostart_sync = FALSE;
#else
	autostart_sync = TRUE;
#endif
	
	
	printf("\n\n\n*** URUCHAMIANIE...\n");
	printf("Enkoder v1.6 2010-2013 Tomek Jaworski\n");
	printf("Kompilacja: %s @ %s\n\n", __DATE__, __TIME__);	
	
	OpenSPI();
	wnd.size = 10;

/*	while(1)
	{
		printf("%c", MARK ? '1' : '0');
		__delay32(5000000);
	}	
*/


	cmd.command = 0x0000;
	cmd.SHDN = 1;
	cmd.GA = 1;
	Imax = 10000; // 10,000 imp/50ms daje 600RPM przy 20,000 imp na obrót
	
	
	sync.pulses_per_revolution = 8;
	sync.delta = PULSES_PER_REVOLUTION_I / sync.pulses_per_revolution;
	sync.pulse_width = 500;

	//sync.power[0].start = 0xFFFF;
	//memset(sync.power, 0x00, sizeof(struct POWER_ANGLE) * 6);
	//memset(sync.power_new, 0x00, sizeof(struct POWER_ANGLE) * 6);

	memset(sync.angles_new, 0x00, sizeof(struct POWER_ANGLE) * ANGLES_PER_CHANNEL);
	memset(channels, 0x00, sizeof(struct POWER_CHANNEL) * 6);
	sync.angles_ready = FALSE;

	int i;
	for (i = 0; i < 10; i++)
	{
		LED4 = 0;
		 __delay32(500000);
		LED4 = 1;
		 __delay32(500000);
	}	
/*
	while(1)
	{
		POWER1 = 1; __delay32(500000); POWER1 = 0;__delay32(500000);
		POWER2 = 1; __delay32(500000); POWER2 = 0;__delay32(500000);
		POWER3 = 1; __delay32(500000); POWER3 = 0;__delay32(500000);
		POWER4 = 1; __delay32(500000); POWER4 = 0;__delay32(500000);
		POWER5 = 1; __delay32(500000); POWER5 = 0;__delay32(500000);
		POWER6 = 1; __delay32(500000); POWER6 = 0;__delay32(500000);
	}	

*/

	// domyslnie generatory sa uruchomione
	POWER_PORT |= power_bits[0] | power_bits[1] |power_bits[2] |power_bits[3] |power_bits[4] |power_bits[5];
	
start:


	WriteSPI(cmd.command);

	IEC0bits.T1IE = FALSE;
	IEC1bits.U2RXIE = FALSE;
	T1.exit_run_command = FALSE;
	T1.fast_position_read = FALSE;
	rs.recv_plus_count = 0;
	sync.enabled = FALSE;


	printf("Ustaw parametr: 'param=wartosc' lub wpisz 'run'\n");
	printf("Zatrzymanie transmisji danych: '+++'\n\n");
	printf("Dostepne polecenia:\n");
	printf("  reset\n\t- restart urzadzenia\n");
	printf("  run\n\t- uruchomienie pomiaru predkosci i pozycji\n");
	printf("  dactest\n\t- tryb testowania przetwornika D/A\n");
	printf("  fpr\n\t- Tryb FAST POSITION READ\n");
	printf("  sync\n\t- Tryb synchronizacji kamery\n");
	printf("  sync test\n\t- Tryb synchronizacji kamery (TEST)\n");
	printf("  set wndsize=xxx (1-64)\n\t- wielkosc okna do usredniania odczytow predkosci\n");
	printf("  set imax=xxxx (0-2^30)\n\t- ilosc impulsow / 50ms przy 600RPM\n");
	printf("  set dac=xxx (0-4095)\n\t- ustawienie wartosci przetrownika D/A\n");
	printf("  set spwidth=xxx (0-65e3)\n\t- dlugosc sygnalu synchronzacyjnego [x100us]\n");
	//printf("  set sdelta=xxx (0-19999)\n\t- przerwa miedzykolejnymi synchronizacjami (imp. enkodera)\n");
	printf("  set scount=xxx (1-16)\n\t- Ustaw liczbe impulsow synchronizacyjnych na obrot\n");
	printf("  set Px (x=1-6)\n\t- Wlaczenie generatora 1-6\n");
	printf("  res Px (x=1-6)\n\t- Wyaczenie generatora 1-6\n");
	printf("  set Vx (x=1-5)\n\t- Wlaczenie zaworu 1-5 (x=9 - wszystkie)\n");
	printf("  res Vx (x=1-5)\n\t- Wyaczenie zaworu 1-5 (x=9 - wszystkie)\n");
	printf("  mark\n\t- Wyœwietla licznik czujnika znaku\n");
	printf("\n");
	
	//#define CMD_EQ(__line, __cmd) (strncmp(__line, (__cmd), strlen(__cmd)) == 0)

	while(TRUE)
	{
		if (autostart_sync)
		{
			autostart_sync = FALSE;
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
		
		if (TestCommand(line, "set~p"))
		{
			int x = line[strlen(line) - 1] - 48; 
			if (x < 1 || x > 6)
			{
				printf("Niepoprawny numer generatora: %d\n", x);
				continue;
			}
			
			POWER_PORT = POWER_PORT | power_bits[x - 1];
			printf("Generator %d wlaczony.\n", x);
			continue;
		}
		
		if (TestCommand(line, "res~p"))
		{
			int x = line[strlen(line) - 1] - 48; 
			if (x < 1 || x > 6)
			{
				printf("Niepoprawny numer generatora: %d\n", x);
				continue;
			}
			
			POWER_PORT = POWER_PORT & ~power_bits[x - 1];
			printf("Generator %d wylaczony.\n", x);
			continue;
		}

		if (TestCommand(line, "set~v"))
		{
			int x = line[strlen(line) - 1] - 48; 
			if ((x < 1 || x > 5) && x != 9)
			{
				printf("Niepoprawny numer zaworu: %d\n", x);
				continue;
			}
			
			SetValve(x, TRUE);
			printf("Zawór %d wlaczony.\n", x);
			continue;
		}
		
		if (TestCommand(line, "res~v"))
		{
			int x = line[strlen(line) - 1] - 48; 
			if ((x < 1 || x > 5) && x != 9)
			{
				printf("Niepoprawny numer zaworu: %d\n", x);
				continue;
			}
			
			SetValve(x, FALSE);
			printf("Zawór %d wylaczony.\n", x);
			continue;
		}
		
		if (TestCommand(line, "mark"))
		{
			printf("Licznik czujnika znaku = %d\n", water.mark_counter);
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

		/*
		if (TestCommand(line, "set~sdelta"))
		{
			printf("Aktualna odleglosc miedzy impulsami synchronizacyjnymi: %d\n", sync.delta);
			sync.delta = atol(strchr(line, '=') + 1);
			printf("Nowa odleglosc miedzy impulsami synchronizacyjnymi: %d\n", sync.delta);
			continue;
		}
		*/	
		
		if (TestCommand(line, "set~scount"))
		{
			printf("Aktualna liczba impulsow na obrot: %d (delta=%d)\n", sync.pulses_per_revolution, sync.delta);
			sync.pulses_per_revolution = atol(strchr(line, '=') + 1);
			sync.delta = PULSES_PER_REVOLUTION_I / sync.pulses_per_revolution;
			printf("Nowa liczba impulsow na obrot: %d (delta=%d)\n", sync.pulses_per_revolution, sync.delta);
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
				if (!sync.data_received)
					continue;
					
				sync.data_received = FALSE;
				asm("nop");
				asm("nop");
				asm("nop");
				asm("nop");
				
				// sync.buffer[0] to '#' - naglowek	
				BYTE CH = (sync.buffer[1] - 'A') - 1;
				BYTE angles = sync.buffer[2] - 'A';

				if (angles == 0)
					continue; // cos jest nie tak

				// sprawdzenie poprawnosci pakietu
				int i;
				for (i = 1; i < 2 + angles*2*4; i++)
					if (sync.buffer[i] < 'A' || sync.buffer[i] > 'P')
						continue; // blad w pakiecie
				
				WORD crc1 = CalcCRC16(sync.buffer + 1, 2 + angles*2*4);
				WORD crc2 = String2WORD(sync.buffer + 1 + 2 + angles*2*4);
				
				if (crc2 != 0xFFFF) // jesli w pakiecie jest 0xFFFF to ignoruj sprawdzanie
					if (crc2 != crc1)
						continue;

				if (CH >= 0 && CH <= 5) // kana³y 1-6 generator
				{
					for (i = 0; i < angles; i++)
					{
						sync.angles_new[i].start = String2WORD(sync.buffer + 3 + i * 2*4 + 0);
						sync.angles_new[i].stop = String2WORD(sync.buffer + 3 + i * 2*4 + 4);
						
						switch (sync.angles_new[i].start)
						{
							case 0xFFFF:
								sync.angles_new[i].mode = POWER_FIXED_ON;
								break;
								
							case 0xFF00:
								sync.angles_new[i].mode = POWER_FIXED_OFF;
								break;
								
							default:
								if (sync.angles_new[i].start <= sync.angles_new[i].stop)
									sync.angles_new[i].mode = POWER_CASE_A; // start <=stop
								else
									sync.angles_new[i].mode = POWER_CASE_B; // start > stop
						} // 			
					}			
					channels[CH].count = angles;
					channels[CH].reload = TRUE;
					sync.angles_ready = TRUE;
					
					putc('@');
				} // generator - kana³ 1-6
				
				if (CH == 9) // KANA£ 10: ustawienie zaworów
				{
					WORD arg1 = String2WORD(sync.buffer + 3 + 0 * 2*4 + 0); // flagi zaworów
					WORD arg2 = String2WORD(sync.buffer + 3 + 0 * 2*4 + 4);	// czas dzia³ania w sekundach
					
					IEC0bits.T1IE = FALSE; // zablokuj przertwanie zegara, które koliduje z w³¹czeniem/wy³¹czeniem zaworów
					
					SetValve(1, !!(arg1 & 0x01));
					SetValve(2, !!(arg1 & 0x02));
					SetValve(3, !!(arg1 & 0x04));
					SetValve(4, !!(arg1 & 0x08));
					SetValve(5, !!(arg1 & 0x10));
					water.valve_off_delay = arg2 << 1;
					water.valve_off_timer = 0;
					
					IEC0bits.T1IE = TRUE; // w³¹cz ponownie przerwanie
					
					putc('@');
				}

				if (CH == 10) // KANA£ 11: pobieranie wartoœci licznika obrotów taœmy
				{
					//WORD arg1 = String2WORD(sync.buffer + 3 + 0 * 2*4 + 0);
					//WORD arg2 = String2WORD(sync.buffer + 3 + 0 * 2*4 + 4);
					printf("$%04x@", water.mark_counter);
				}
				
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
	
//	LCDClearScreen();
	
	while(TRUE)
	{
		if (T1.recalc_position)
		{
			T1.recalc_position = FALSE;
			
			T1.position = (float)T1.encoder_pos;
			T1.position *= ROLLER_POS_COEF;

			sprintf(T1.str_position, "%.1f", T1.position);
		//	LCDGotoXY(0, 1);
		//	LCDWriteString("P=");
		//	LCDWriteString(T1.str_position);		
		//	LCDWriteString("cm  ");			
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
				
		//	LCDGotoXY(0, 0);
		//	LCDWriteString("V=");
		//	LCDWriteString(T1.str_velocity);		
		//	LCDWriteString("rpm  ");
		}
		
		if (T1.exit_run_command)
			goto start;	
	}	
	
	return 0;
}


#define _AUTOPSV	__attribute__((auto_psv))
#define _NOAUTOPSV	__attribute__((no_auto_psv))

BOOL do_switch;
void _ISR _NOAUTOPSV _T1Interrupt(void) // co 50ms
{
	IFS0bits.T1IF = FALSE;
	

	if (water.counter500ms >= 5000)
	{
		water.counter500ms = 0;
		water.timer_500ms++;
		
		
		water.valve_off_timer++;
		if (water.valve_off_delay > 0 && water.valve_off_timer > water.valve_off_delay)
		{
			water.valve_off_delay = 0;
			SetValve(9, FALSE);
		}	
	}
	water.counter500ms++;
	
	
		
	
	WORD delta;	
	T1.encoder_pos = POSCNT;
	
	WORD i;
	if (sync.enabled)
	{
		// liczniki czasów trwania obu impulsow synchronizacyjnych
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
		
		// sprawdzenie, ktory impuls teraz poleci
		if (T1.encoder_pos >= sync.counter)
		{
			sync.sync0_counter = sync.pulse_width;
			SYNC_MINOR = SYNC_HIGH;
			sync.counter += sync.delta;
		}	
	
		
		for (i = 0; i < 6; i++)
		{
			do_switch = FALSE;
			if (channels[i].current_angle.mode == POWER_CASE_A) // start < stop
			{
				if ((T1.encoder_pos >= channels[i].current_angle.start) && (T1.encoder_pos < channels[i].current_angle.stop))
				{
					POWER_PORT |= power_bits[i]; 
					//POWER1 = 1;
					channels[i].power_on = TRUE;
				} else
					if (channels[i].power_on)
					{
						//POWER1 = 0;
						POWER_PORT = POWER_PORT & ~power_bits[i];
						do_switch = TRUE;
					}	
					
						
			} else
				if (channels[i].current_angle.mode == POWER_CASE_B) // start > stop 
				{
					
					if ((T1.encoder_pos >= channels[i].current_angle.start) || (T1.encoder_pos < channels[i].current_angle.stop))
					{
						//POWER1 = 1;
						POWER_PORT |= power_bits[i];
						channels[i].power_on = TRUE;
					} else
						if (channels[i].power_on)
						{
							//POWER1 = 0;
							POWER_PORT = POWER_PORT & ~power_bits[i];
							do_switch = TRUE;
						}	
						
		
		
				} else
					if (channels[i].current_angle.mode == POWER_FIXED_ON)
						POWER_PORT |= power_bits[i];
					else
						POWER_PORT = POWER_PORT & ~power_bits[i]; // POWER_FIXED_OFF
						//POWER1 = 0; // power off
			
			if (do_switch)
			{
				channels[i].current_angle = *channels[i].pointer;
				channels[i].pointer++;
				
				channels[i].power_on = FALSE;
				if (channels[i].pointer == channels[i].end)
					channels[i].pointer = channels[i].angles;
			}	
		} // for
		
		
		
		// jesli sa nowe dane o katach zaplonu, to skorzystaj z nich
		if (sync.angles_ready)
		{
			//memcpy(sync.power, sync.power_new, sizeof(struct POWER_ANGLE) * 6);
			sync.angles_ready = FALSE;
			
			for (i = 0; i < 6; i++)
			{
				if (!channels[i].reload)
				continue;

				channels[i].reload = FALSE;
				memcpy(channels[i].angles, sync.angles_new, sizeof(struct POWER_ANGLE) * channels[i].count);
				channels[i].pointer = channels[i].angles;
				channels[i].end = channels[i].angles + channels[i].count;
				//POWER1 = 0;
				POWER_PORT = POWER_PORT & ~power_bits[i];
				
				// do switch
				channels[i].current_angle = *channels[i].pointer;
				channels[i].pointer++;
			
				if (channels[i].pointer == channels[i].end)
					channels[i].pointer = channels[i].angles;
			}	
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

//void _ISR _NOAUTOPSV _Interrupt58(void)//_QEIInterrupt(void)
void _ISR _NOAUTOPSV _QEIInterrupt(void)//_QEIInterrupt(void)
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
	
	if (sync.enabled)
	{
		if (byte == '#')
		{
			sync.buffer_pos = 0;
			sync.data_received = FALSE;
		}	
			
		if (sync.buffer_pos < RECV_BUFFER_SIZE - 1)
		{
			sync.buffer[sync.buffer_pos] = byte;
			sync.buffer_pos++;
			//if (sync.buffer_pos == 1+6*2*4+1*4)
			//	sync.data_received = TRUE;
		}	
	
		if (byte == '%')
		{
			sync.data_received = TRUE;
		}	
	}	
	
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

void _ISR _AUTOPSV _INT2Interrupt(void)
{
	IFS1bits.INT2IF = FALSE;
	
	if (water.timer_500ms >= 2) // przerwanie min co sekundê
	{
		//printf("t=%d\n", water.timer_500ms);
		water.mark_counter++;
		water.timer_500ms = 0;
	}	
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


void SetValve(int valve, int state)
{
	if (valve == 9) // w³¹cz/wy³¹cz wszystkie
	{
		VALVE1 = state;
		VALVE2 = state;
		VALVE3 = state;
		VALVE4 = state;
		VALVE5 = state;
	}
	
	// w³¹cz/wy³¹cz pojedyncze zawory
	if (valve == 1)
		VALVE1 = state;
	if (valve == 2)
		VALVE2 = state;
	if (valve == 3)
		VALVE3 = state;
	if (valve == 4)
		VALVE4 = state;
	if (valve == 5)
		VALVE5 = state;
}	
