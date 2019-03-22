#if !defined(_SPI_SW_H_)
#define _SPI_SW_H_


#define SPI_CS			LATFbits.LATF3 // pin 33 -> 2
#define SPI_SCK			LATFbits.LATF2 // pin 34 -> 3
#define SPI_SDO			LATFbits.LATF6 // pin 35 -> 4

#define SPI_TRIS_CS		TRISFbits.TRISF3
#define SPI_TRIS_SCK	TRISFbits.TRISF2
#define SPI_TRIS_SDO	TRISFbits.TRISF6

void OpenSPI(void);
void WriteSPI(WORD value);

union __DAC_COMMAND
{
	WORD command;
	struct
	{
		unsigned value	: 12;
		unsigned SHDN	: 1;
		unsigned GA		: 1;
		unsigned BUF	: 1;
		unsigned AB		: 1;
	};	
};	

typedef union __DAC_COMMAND DAC_COMMAND;

#endif // _SPI_SW_H_
