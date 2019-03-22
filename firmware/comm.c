#include <p33FJ128MC706A.h>
#include <stddef.h>

#include <generic.h>
#include "comm.h"

void putc(char c)
{
	while(U2STAbits.UTXBF);
    U2TXREG = c;
}

char getc(void)
{
	while(!DataRdyUART2());
	return U2RXREG;
}	

char* gets_safe(char* str, WORD size, BOOL echo)
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
			if (echo)
				putc('\n');
			return str;
		}
		
		if (echo)
			putc(c);
			
		if (size-- == 0)
		{
			*ptr = '\x0';
			if (echo)
				putc('\n');
			return str;
		}		
		
		*ptr = c;
		ptr++;
		
	}
}
