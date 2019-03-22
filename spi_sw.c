#include "defs.h"
#include "spi_sw.h"

#define CS	SPI_CS
#define DO	SPI_SDO
#define CLK	SPI_SCK

//#define nop1 asm("nop\n");
//#define nop2 asm("nop\nnop\n");
//#define nop3 asm("nop\nnop\nnop\n");
#define nop4 asm volatile ("nop\nnop");

//#define nop4  asm volatile("nop\nnop\nnop\nnop\n");asm volatile("nop\nnop\nnop\nnop\n");asm volatile("nop\nnop\nnop\nnop\n");asm volatile("nop\nnop\nnop\nnop\n");asm volatile("nop\nnop\nnop\nnop\n");

void OpenSPI(void)
{		
	SPI_TRIS_CS = 0;
	SPI_TRIS_SDO = 0;
	SPI_TRIS_SCK = 0;

	CS = 1;
	CLK = 0;
	DO = 0;
	
	__delay32(5000); // 500us
		
}

void WriteSPI(WORD value)
{
	WORD_VAL w;
	w.Val = value;
	
	CS = 0;
	int i;
	for (i = 0; i < 16; i++)
	{
		if (w.bits.b15)
			DO = 1;
		else
			DO = 0;
			
		nop4;
		CLK = 1;
		nop4;
		CLK = 0;
		
		w.Val <<= 1;
	}
	CS = 1;
	nop4;
	DO = 0;
}


