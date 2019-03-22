#include "comm.h"

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
