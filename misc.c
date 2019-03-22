#include <p33FJ128MC706A.h>
#include <Generic.h>
#include <timer.h>
#include <qei.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

int isXdigit(char x)
{
	if (x >= '0' && x <= '9')
		return TRUE;
	if (x >= 'A' && x <= 'F')
		return TRUE;
	
	return FALSE;
}	

WORD CalcCRC16(const char* buffer, WORD cnt)
{
	WORD crc = 0;
	while(cnt)
	{
		crc ^= (WORD)(*buffer);
		crc <<= 1;
		buffer++;
		cnt--;;
	}
	
	return crc;
}

	
WORD String2Hex16(const char* str)
{
	WORD result = 0x0000;
	
	int i;
	for (i = 0; i < 4; i++)
	{
		result <<= 4;
		
		if (str[i] >= 'A')
			result |= (str[i] - 'A') + 10;
		else
			result |= (str[i] - '0');
	}
	
	return result;	
}


	
WORD String2WORD(const char* str)
{
	WORD result = 0x0000;
	
	int i;
	for (i = 0; i < 4; i++)
	{
		result <<= 4;
		result |= (str[i] - 'A');
	}
	
	return result;	
}
	
