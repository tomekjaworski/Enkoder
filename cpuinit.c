#include "defs.h"


void InitCPU(void)
{
	SET_CPU_IPL(0x07);
	
	AD1PCFGL = 0xFFFF; // tylko porty cyfrowe
	AD2PCFGL = 0xFFFF; // tylko porty cyfrowe
	
	//LATA = 0;
	LATB = 0;
	LATC = 0;
	LATD = 0;
	LATE = 0;
	LATF = 0;
	LATG = 0;
	
	TRISB = 0b0000000100111000; // CTS,QC,QB,QA
	TRISC = 0b0000000000000000;
	TRISD = 0b0000000000000000;
	TRISE = 0b0000000000000000;
	TRISF = 0b0000000000010000; // RXD
	TRISG = 0x0000;
	
	
	// timer 1
	TMR1 = 0x0000;
	PR1 = TIMER1_50MS; // timer1 co 50ms
	T1CON = 0x0000;
	T1CONbits.TON = TRUE;
	T1CONbits.TCKPS = 0b01; // input clock prescale = 1:8
	IPC0bits.T1IP = 5;
	
	IFS0bits.T1IF = FALSE;
	IEC0bits.T1IE = FALSE;
	
	
	// UART 2
	// Fosc = 20MHz
	// Fcy = Fosc/2;
	
	// ### BRGH = 0
	// UxBRG = ((Fcy / BaudRate) / 16 - 1)
	// 9600 = 64
	// 19200 = 32
	
	// ### BRGH = 1
	// UxBRG = [Fcy / (4xBaudRate)] - 1
	// 9600 = 259
	// 19200 = 129
	// 57600 = 42
	// 115200 = 21
	U2MODE = 0x0000;
	U2BRG = 21;
	U2MODEbits.BRGH = TRUE;
	
	U2MODEbits.UARTEN = TRUE;
	U2STAbits.UTXEN = TRUE;
	
	IEC1bits.U2TXIE = FALSE;
	IEC1bits.U2RXIE = FALSE;
	IEC4bits.U2EIE = FALSE;
	
	IFS1bits.U2TXIF = FALSE;
	IFS1bits.U2RXIF = FALSE;
	IFS4bits.U2EIF = FALSE;
	
	__C30_UART = 2;
	/*
		IFS1bits.U2TXIF
		IFS1bits.U2RXIF
		IFS4bits.U2EIF // error interrupt flag
		
		IEC1bits.U2TXIE
		IEC1bits.U2RXIE
		IEC4bits.U2EIE
		
	*/
	
	//PMD1bits.QEI1MD = 0;
	QEICON = 0x0000;
	QEICONbits.QEIM = 0b111; // 100
	QEICONbits.POSRES = TRUE;
	QEICONbits.SWPAB = TRUE;
	DFLTCONbits.CEID = TRUE;
	IEC3bits.QEIIE = TRUE;
	MAXCNT = PULSES_PER_REVOLUTION_I - 1;
	
	
	SET_CPU_IPL(0x00);

	LED3 = TRUE;
	LED4 = TRUE;
	LED5 = TRUE;
	
	SYNC_MINOR = SYNC_LOW;
	SYNC_REVOLUTION = SYNC_LOW;

	
	
}	

