#include "defs.h"

void HardwareLock(void)
{
//	OSCCON = 0x46;
//	OSCCON = 0x57;
//	OSCCONbits.IOLOCK = TRUE;
}

void HardwareUnlock(void)
{
//	OSCCON = 0x46;
//	OSCCON = 0x57;
//	OSCCONbits.IOLOCK = FALSE;
}

void InitCPU(void)
{
	SET_CPU_IPL(0x07);
	
	HardwareUnlock();
		
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
	
	// dla timera:
	// F = 20MHz;
	// Fcy = F/2;
	// Ft1 = Fcy / 64; (preskaler);
	// Tt1 = 1/Ft1;
	// Tx = 100ms = 0.1s
	// PR1 = Tx / Tt1;
	
	// timer 1
	TMR1 = 0x0000;
	PR1 = 15625; // timer1 co 100ms
	T1CON = 0x0000;
	T1CONbits.TON = TRUE;
	T1CONbits.TCKPS = 0b10; // input clock prescale = 1:64
	
	IFS0bits.T1IF = FALSE;
	IEC0bits.T1IE = TRUE;
	
	
	// UART 2
	// Fosc = 20MHz
	// Fcy = Fosc/2;
	
	// ### BRGH = 0
	// UxBRG = ((Fcy / BaudRate) / 16 - 1)
	// 9600 = 64
	// 19200 = 32
	
	// ### BRGH = 1
	// UxBRG = Fcy / (4xBaudRate) - 1
	// 9600 = 259
	// 19200 = 129
	// 57600 = 42
	U2MODE = 0x0000;
	U2BRG = 129;
	U2MODEbits.BRGH = TRUE;
	
	U2MODEbits.UARTEN = TRUE;
	U2STAbits.UTXEN = TRUE;
	
	
	//PMD1bits.QEI1MD = 0;
	QEICON = 0x0000;
	QEICONbits.QEIM = 0b110; // 100
	QEICONbits.POSRES = TRUE;
	DFLTCONbits.CEID = TRUE;
	IEC3bits.QEIIE = TRUE;
	
	
	SET_CPU_IPL(0x00);


	
	
}	

