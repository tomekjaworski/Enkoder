#if !defined(_COMM_H_)
#define _COMM_H_

#include <uart.h>

#undef putc
#undef getc

void putc(char c);
char* gets_safe(char* str, WORD size, BOOL echo);
char getc(void);

#define KEYPRESSED (DataRdyUART2())

#endif //_COMM_H_
