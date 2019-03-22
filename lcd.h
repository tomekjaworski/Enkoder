#if !defined(_LCD_H_)
#define _LCD_H_


void LCDClearScreen(void);
void LCDInit(void);
void LCDWriteString(const char *str);
void LCDGotoXY(unsigned char X, unsigned char Y);
void LCDCursorOff(void);
void LCDCursorOn(BOOL block);


#endif // _LCD_H_

